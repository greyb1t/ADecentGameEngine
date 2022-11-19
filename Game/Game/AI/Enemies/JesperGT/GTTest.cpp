#include "pch.h"
#include "GTTest.h"

#include "BTN_GTChasePlayer.h"
#include "BTN_GTShoot.h"
#include "AI/BehaviorTree/Base/BT_Builder.h"
#include "AI/BehaviorTree/Base/BT_Inverter.h"
#include "AI/BehaviorTree/Base/BT_Selector.h"
#include "AI/BehaviorTree/Base/BT_Sequence.h"
#include "AI/Enemies/Base/EnemyScalableStats.h"
#include "AI/GeneralNodes/BTN_FlyTowardsPlayer.h"
#include "AI/GeneralNodes/BTN_InRangeOfPlayer.h"
#include "AI/GeneralNodes/BTN_MoveTowardsPlayer.h"
#include "AI/GeneralNodes/BTN_Shoot.h"
#include "AI/Health/EnemyHealth.h"
#include "Engine/Shortcuts.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "Engine/GameObject/Components/AudioComponent.h"
#include "Engine/GameObject/Components/CharacterControllerComponent.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/Reflection/Reflector.h"
#include "Engine/VFX/VFXUtilities.h"

GTTest::GTTest(GameObject* aGameObject) :
	EnemyBase(aGameObject)
{
}

void GTTest::Start()
{
	EnemyBase::Start();


	SetEnemyShootType(eEnemyShootType::GroundPopcorn);
	const Vec3f playerpos = myGameObject->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos();

	auto& scalableStats = myGameObject->GetComponent<EnemyScalableStats>()->GetScalableValues();

	auto deathFunction = [&]()
	{
		cct->SetGravity(982.f);
		myBlackboard.setBool("IsDead", true);
		cct->Destroy();
	};

	myEnemyHealth = myGameObject->AddComponent<EnemyHealth>(scalableStats.myHealth);
	myEnemyHealth->Init(&myBlackboard, deathFunction);

	cct = myGameObject->AddComponent<Engine::CharacterControllerComponent>();
	cct->Init(myColliderHeight, myColliderRadius);
	cct->SetMovementSettings(.01f);
	cct->SetOffset(Vec3f(0.f, myColliderOffSetY, 0.f));
	cct->SetGravity(982.f);

	myGameObject->SetLayers(eLayer::ENEMY | eLayer::DAMAGEABLE, eLayer::DEFAULT, eLayer::DEFAULT);
	myGameObject->SetTag(eTag::ENEMY);

	myAnimationController = &GetGameObject()->GetComponent<Engine::AnimatorComponent>()->GetController();

	myBlackboard.SetGameObject(GetGameObject());
	myBlackboard.SetAnimationController(myAnimationController);
	myShootBoneIndex = myBlackboard.GetAnimationController()->GetBoneIndexFromName("Head");

	myBTree = BT_Builder()
		.composite<BT_Selector>()
			.composite<BT_Sequence>()
				.leaf<BTN_InRangeOfPlayer>(&myBlackboard)
				.leaf<BTN_GTShoot>(&myBlackboard, this)
			.end()
			.composite<BT_Sequence>()
				.decorator<BT_Inverter>()
					.leaf<BTN_InRangeOfPlayer>(&myBlackboard)
				.end()
				.leaf<BTN_GTChasePlayer>(&myBlackboard, this)
			.end()
		.end()
		.build();

	InitAudio();
	InitAnimationCallbacks();
}

void GTTest::Execute(Engine::eEngineOrder aOrder)
{
	EnemyBase::Execute(aOrder);// Mandatory to be able to work correctly, do not remove!

	Update(Time::DeltaTime); // Mandatory to be able to work correctly, do not remove!

	myAnimationController->SetFloat("Movement", 2.f);
}

void GTTest::Update(float aDeltaTime)
{
	if (myBlackboard.getBool("IsDead"))
		return;

	if (!myHasFinsihedSpawning)
		return;

	{ // Update head collider pos follows
		Mat4f boneTrans = myAnimationController->GetBoneTransformWorld(myShootBoneIndex);
		Vec3f translation;
		Quatf rotation;
		Vec3f scale;
		boneTrans.Decompose(translation, rotation, scale);
		myBlackboard.setVec3f("ShootPosition", translation);

		GDebugDrawer->DrawSphere3D(
			DebugDrawFlags::AI,
			translation,
			50.f,
			0.f,
			{ 1.f, 0.f, 1.f, 1.f },
			false);
	}

	myBTree.update();
}

void GTTest::Reflect(Engine::Reflector& aReflector)
{
	EnemyBase::Reflect(aReflector);

	aReflector.Reflect(myBlackboard.GetGroundPopcornShootData(), "Shoot Data");

	aReflector.Reflect(myShootingRange, "Shoot Range");
	myBlackboard.setFloat("ShootingRange", myShootingRange);

	aReflector.Reflect(myColliderHeight, "Collider Height");
	aReflector.Reflect(myColliderRadius, "Collider Radius");
	aReflector.Reflect(myColliderOffSetY, "Collider OffSet Y");

	aReflector.Reflect(myMoveSpeed, "Move Speed");
	myBlackboard.setFloat("MoveSpeed", myMoveSpeed);
}

void GTTest::OnDamage()
{
	GetBlackboard().GetAudioComponent()->PlayEvent("Hurt");
}

BT_Blackboard& GTTest::GetBlackboard()
{
	return myBlackboard;
}

void GTTest::InitAnimationCallbacks()
{
	//myBlackboard.GetAnimationController()->AddEventCallback(
	//	"Footstep_PopcornGround",
	//	[this]()
	//	{
	//		myAudioComponent->PlayEvent("Run"); // SoundImplementation LabCoatRun
	//	}
	//);


	myAnimationController->AddStateOnExitCallback(
		"Base Layer",
		"Spawn",
		[this]()
		{
			myHasFinsihedSpawning = true;
		}
	);
}

void GTTest::Render()
{
	EnemyBase::Render();
	DebugDraw();
}

void GTTest::DebugDraw()
{
	if (myBlackboard.hasFloat("ShootingRange"))
	{
		GDebugDrawer->DrawSphere3D(
			DebugDrawFlags::AI,
			myGameObject->GetTransform().GetPosition(),
			myBlackboard.getFloat("ShootingRange"),
			0.f,
			{ 1.f, 0.f, 1.f, 1.f });
	}
}

void GTTest::InitAudio()
{
	myAudioComponent = myGameObject->AddComponent<Engine::AudioComponent>();
	myAudioComponent->AddEvent("Death", "event:/SFX/NPC/GROUNDCORN/Death");
	myAudioComponent->AddEvent("Hurt", "event:/SFX/NPC/GROUNDCORN/Hurt");
	myAudioComponent->AddEvent("Projectile", "event:/SFX/NPC/GROUNDCORN/Projectile");
	myAudioComponent->AddEvent("Run", "event:/SFX/NPC/GROUNDCORN/Run");

	myBlackboard.SetAudioComponent(myAudioComponent);
}