#include "pch.h"
#include "Runner.h"

#include "BTN_Runner_MoveTowardsPosition.h"
#include "BTN_Runner_Rotate.h"
#include "BTN_SuicideExplode.h"
#include "AI/BehaviorTree/Base/BT_Builder.h"
#include "AI/BehaviorTree/Base/BT_Inverter.h"
#include "AI/BehaviorTree/Base/BT_Selector.h"
#include "AI/BehaviorTree/Base/BT_Sequence.h"
#include "AI/Enemies/Base/EnemyScalableStats.h"
#include "AI/Explosion/GeneralExplosion.h"
#include "AI/GeneralNodes/BTN_FlyTowardsPlayer.h"
#include "AI/GeneralNodes/BTN_InRangeOfPosition.h"
#include "AI/GeneralNodes/BTN_LambdaCall.h"
#include "AI/GeneralNodes/BTN_MoveTowardsPosition.h"
#include "AI/Health/EnemyHealth.h"
#include "AI/VFX/EnemyVFXRef.h"
#include "Engine/Shortcuts.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/AudioComponent.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "Engine/GameObject/Components/CameraShakeComponent.h"
#include "Engine/GameObject/Components/CharacterControllerComponent.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/Reflection/Reflector.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"

Runner::Runner(GameObject* aGameObject) :
	EnemyBase(aGameObject)
{
}

void Runner::Start()
{
	EnemyBase::Start();

	const Vec3f playerpos = myGameObject->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos();

	auto& scalableStats = myGameObject->GetComponent<EnemyScalableStats>()->GetScalableValues();

	{
		myFuseGO = myGameObject->GetScene()->AddGameObject<GameObject>();
		//myFuseGO->GetTransform().SetParent(&GetTransform());
		myFuseGO->GetTransform().SetPositionLocal(Vec3f(0.f, 0.f, 0.f));

		const auto& vfxRef = myGameObject->GetSingletonComponent<EnemyVFXRef>()->GetmyRunnerFuse();
		myVFX = myFuseGO->AddComponent<Engine::VFXComponent>(vfxRef->Get());
		myVFX->Play();
		myVFX->AutoDestroy();
	}

	std::function<void()> deSpawnFunction = [this]()
	{
		myGameObject->Destroy();
	};
	SetDeSpawnFunction(deSpawnFunction);

	std::function<void()> deathFunction = [this]()
	{
		//myBlackboard.GetAudioComponent()->PlayEvent("Explode");
		SpawnDeathSound();
		myBlackboard.GetAudioComponent()->StopEvent("Fuse");
		cct->Destroy();
		const Vec3f ownerPos = myGameObject->GetTransform().GetPosition();
		GDebugDrawer->DrawSphere3D(
			DebugDrawFlags::AI,
			ownerPos,
			myBlackboard.getFloat("ExplosionRadius"),
			0.5f,
			{ 1.f, 0.f, 1.f, 1.f });

		const float distance = (myGameObject->GetScene()->GetMainCamera().GetTransform().GetPosition() - myGameObject->GetTransform().GetPosition()).Length();

		const auto mainCam = myGameObject->GetScene()->GetMainCameraGameObject();
		if (const auto shakeComponent = mainCam->GetComponent<Engine::CameraShakeComponent>())
		{
			shakeComponent->AddPerlinShakeByDistance("Normal Enemy Death", 5000.f, distance);
		}

		auto efgr = myBlackboard.getFloat("ExplosionRadius");

		std::vector<OverlapHit> hits;
		myGameObject->GetScene()->SphereCastAll(ownerPos, myBlackboard.getFloat("ExplosionRadius"), eLayer::PLAYER, hits);
		for (size_t i = 0; i < hits.size(); i++)
		{
			if (hits[i].GameObject->GetLayer() & eLayer::ENEMY)
			{
				int hej = 7;
			}

			if (!hits[i].GameObject)
				continue;

			auto health = hits[i].GameObject->GetComponent<HealthComponent>();

			if (health)
			{
				auto& scalableStats = myGameObject->GetComponent<EnemyScalableStats>()->GetScalableValues();
				health->ApplyDamage(scalableStats.myDamage);
			}
		}

		cct->SetGravity(982.f);
		myBlackboard.setBool("IsDead", true);
		myGameObject->GetSingletonComponent<PollingStationComponent>()->EnemyCounterDecrease();
		myGameObject->Destroy();
		myFuseGO->Destroy();

		auto explosionObj = myGameObject->GetScene()->AddGameObject<GameObject>();
		//explosionObj->GetTransform().SetRotationLocal(Vec3f(1.f, 0.f, 0.f)); // shit solution, X positive rotates Y positive ;)
		explosionObj->GetTransform().SetPosition(myGameObject->GetTransform().GetPosition() + Vec3f(0.f, 150.f, 0.f));
		explosionObj->AddComponent<GeneralExplosion>(myGameObject->GetSingletonComponent<EnemyVFXRef>()->GetRunnerExplode()->Get());

		myEnemyHealth->GetHealthComponent()->ApplyDamage(10000000000.f);
	};
	myEnemyHealth = myGameObject->AddComponent<EnemyHealth>(scalableStats.myHealth);
	myEnemyHealth->Init(&myBlackboard, deathFunction);

	cct = myGameObject->AddComponent<Engine::CharacterControllerComponent>();
	cct->Init(myColliderHeight, myColliderRadius);
	cct->SetMovementSettings(.01f);
	cct->SetOffset(Vec3f(0.f, myColliderOffSetY, 0.f));
	cct->SetGravity(982.f);

	myGameObject->SetLayers(eLayer::ENEMY | eLayer::ENEMY_DAMAGEABLE, eLayer::DEFAULT, eLayer::DEFAULT);
	myGameObject->SetTag(eTag::ENEMY);

	myAnimationController = &GetGameObject()->GetComponent<Engine::AnimatorComponent>()->GetController();

	//myBlackboard = BT_Blackboard(GetGameObject(), myAnimationController);
	myBlackboard.SetGameObject(GetGameObject());
	myBlackboard.SetAnimationController(myAnimationController);

	myFuseIndex = myBlackboard.GetAnimationController()->GetBoneIndexFromName("FuseSocket");

	/*Vec3f hitPos;
	RayCastHit hit;
	if (myGameObject->GetScene()->RayCast(
		playerpos + Vec3f(0.f, 500.f, 0.f),
		Vec3f(0.f, -1.f, 0.f),l
		10000.f,
		eLayer::ALL,
		hit))
	{
		if (hit.GameObject != nullptr)
			myBlackboard.setVec3f("SuicidePosition", hitPos);
		else
			myBlackboard.setVec3f("SuicidePosition", playerpos);
	}
	else
		myBlackboard.setVec3f("SuicidePosition", playerpos);*/

	Vec3f suicidePos = playerpos;
	suicidePos.y = myGameObject->GetTransform().GetPosition().y;
	myBlackboard.setVec3f("SuicidePosition", suicidePos);

	myBlackboard.setBool("CanStartRun", false);
	myBlackboard.setBool("IsDead", false);

	myBTree = BT_Builder()
		.composite<BT_Selector>()
		.leaf<BTN_Runner_Rotate>(&myBlackboard, myBlackboard.getVec3f("SuicidePosition"), this)
		.composite<BT_Sequence>()
		.leaf<BTN_InRangeOfPosition>(&myBlackboard, myBlackboard.getVec3f("SuicidePosition"), 300.f)
		.leaf<BTN_LambdaCall>(deathFunction)
		.end()
		.composite<BT_Sequence>()
		.decorator<BT_Inverter>()
		.leaf<BTN_InRangeOfPosition>(&myBlackboard, myBlackboard.getVec3f("SuicidePosition"), 300.f)
		.end()
		.leaf<BTN_Runner_MoveTowardsPosition>(&myBlackboard, myBlackboard.getVec3f("SuicidePosition"), this, deathFunction)
		.end()
		.end()
		.build();

	InitAnimationCallbacks();
	InitAudio();

	myBlackboard.GetAudioComponent()->PlayEvent("Spawn");
}

void Runner::Execute(Engine::eEngineOrder aOrder)
{
	EnemyBase::Execute(aOrder);// Mandatory to be able to work correctly, do not remove!

	Update(Time::DeltaTime); // Mandatory to be able to work correctly, do not remove!
}

void Runner::Update(float aDeltaTime)
{
	if (myBlackboard.getBool("IsDead"))
		return;

	{
		const Vec3f playerpos = myGameObject->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos();
		myBlackboard.setVec3f("SuicidePosition", playerpos);
	}

	{ // Update head collider pos follows
		Mat4f boneTrans = myAnimationController->GetBoneTransformWorld(myFuseIndex);
		Vec3f translation;
		Quatf rotation;
		Vec3f scale;
		boneTrans.Decompose(translation, rotation, scale);
		myFuseGO->GetTransform().SetPosition(translation);

		GDebugDrawer->DrawSphere3D(
			DebugDrawFlags::AI,
			translation,
			50.f,
			0.f,
			{ 1.f, 0.f, 1.f, 1.f },
			false);
	}

	myBTree.update();
	DebugDraw();
}

void Runner::Reflect(Engine::Reflector& aReflector)
{
	EnemyBase::Reflect(aReflector);

	aReflector.Reflect(mySpeed, "MoveSpeed");
	myBlackboard.setFloat("Speed", mySpeed);

	aReflector.Reflect(myExplosionRadius, "Explosion Radius");
	myBlackboard.setFloat("ExplosionRadius", myExplosionRadius);

	aReflector.Separator();
	aReflector.Reflect(myColliderHeight, "Collider Height");
	aReflector.Reflect(myColliderRadius, "Collider Radius");
	aReflector.Reflect(myColliderOffSetY, "Collider OffSet Y");
}

BT_Blackboard& Runner::GetBlackboard()
{
	return myBlackboard;
}

void Runner::InitAnimationCallbacks()
{
	myBlackboard.GetAnimationController()->AddEventCallback(
		"Footstep_Runner",
		[this]()
		{
			myBlackboard.GetAudioComponent()->PlayEvent("Run");
		}
	);


	/*myAnimationController->AddStateOnExitCallback(
		"Base Layer",
		"Aggro",
		[this]()
		{
			myBlackboard.setBool("AggroAnimOnExitCallback", true);
		}
	);*/

	myAnimationController->AddStateOnExitCallback(
		"Base Layer",
		"Spawn",
		[this]()
		{
			myBlackboard.setBool("CanStartRun", true);
		}
	);
}

void Runner::Render()
{
	EnemyBase::Render();
	/*DebugDraw();*/
}

void Runner::DebugDraw()
{

	/*GDebugDrawer->DrawSphere3D(
		DebugDrawFlags::AI,
		aggroPos,
		myBlackboard.getFloat("AggroRange"),
		0.f,
		{ 1.f, 0.f, 0.f, 1.f });*/

	GDebugDrawer->DrawSphere3D(
		DebugDrawFlags::AI,
		myBlackboard.getVec3f("SuicidePosition"),
		200.f,
		0.f,
		{ 1.f, 0.f, 1.f, 1.f });
}

void Runner::InitAudio()
{
	myAudioComponent = myGameObject->AddComponent<Engine::AudioComponent>();
	myAudioComponent->AddEvent("Explode", "event:/SFX/NPC/RUNNERS/Explode");
	myAudioComponent->AddEvent("Hurt", "event:/SFX/NPC/RUNNERS/Hurt");
	myAudioComponent->AddEvent("Run", "event:/FOL/NPC/Light");
	myAudioComponent->AddEvent("Fuse", "event:/SFX/NPC/RUNNERS/Fuse");
	myAudioComponent->AddEvent("Spawn", "event:/SFX/NPC/RUNNERS/Spawn");

	myAudioComponent->PlayEvent("Fuse");

	myBlackboard.SetAudioComponent(myAudioComponent);
}
void Runner::OnDamage()
{
	GetBlackboard().GetAudioComponent()->PlayEvent("Hurt");
}

void Runner::SpawnDeathSound()
{
	auto gO = myGameObject->GetScene()->AddGameObject<GameObject>();
	gO->GetTransform().SetPosition(myGameObject->GetTransform().GetPosition());

	auto audio = gO->AddComponent<Engine::AudioComponent>();
	gO->AddComponent<Engine::DestructComponent>(5);
	audio->AddEvent("Explode", "event:/SFX/NPC/RUNNERS/Explode");
	audio->PlayEvent("Explode");
}
