#include "pch.h"
#include "Mortar.h"

#include "BTN_MortarIdle.h"
#include "BTN_Mortar_InRangeOfPlayer.h"
#include "BTN_Mortar_OutRangeOfPlayer.h"
#include "BTN_Mortar_Shoot.h"

#include "AI/BehaviorTree/Base/BT_Builder.h"
#include "AI/BehaviorTree/Base/BT_Inverter.h"
#include "AI/BehaviorTree/Base/BT_Selector.h"
#include "AI/BehaviorTree/Base/BT_Sequence.h"
#include "AI/Enemies/Base/EnemyScalableStats.h"
#include "AI/GeneralNodes/BTN_FlyTowardsPlayer.h"
#include "AI/GeneralNodes/BTN_InRangeOfPlayer.h"
#include "AI/GeneralNodes/BTN_LambdaCall.h"
#include "AI/GeneralNodes/BTN_MoveTowardsPlayer.h"
#include "AI/GeneralNodes/BTN_Shoot.h"
#include "AI/Health/EnemyHealth.h"
#include "Engine/Shortcuts.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "Engine/GameObject/Components/AudioComponent.h"
#include "Engine/GameObject/Components/CharacterControllerComponent.h"
#include "Engine/Reflection/Reflector.h"

Mortar::Mortar(GameObject* aGameObject) :
	EnemyBase(aGameObject)
{
}

void Mortar::Start()
{
	EnemyBase::Start();

	SetEnemyShootType(eEnemyShootType::Mortar);
	myGameObject->SetName("Mortar");
	const Vec3f playerpos = myGameObject->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos();

	auto& scalableStats = myGameObject->GetComponent<EnemyScalableStats>()->GetScalableValues();

	std::function<void()> deSpawnFunction = [this]()
	{
		myGameObject->Destroy();
	};
	SetDeSpawnFunction(deSpawnFunction);


	std::function<void()> deathFunction = [this]()
	{
		myBlackboard.GetAudioComponent()->PlayEvent("Death");
		//cct->SetGravity(982.f);
		cct->Destroy();
		myBlackboard.setBool("IsDead", true);
		Enemy::SetEnemyCondition(*myAnimationController, "IsDead", true);
	};
	myEnemyHealth = myGameObject->AddComponent<EnemyHealth>(scalableStats.myHealth);
	myEnemyHealth->Init(&myBlackboard, deathFunction);

	cct = myGameObject->AddComponent<Engine::CharacterControllerComponent>();
	cct->Init(myColliderHeight, myColliderRadius);
	cct->SetMovementSettings(.01f);
	cct->SetOffset(Vec3f(0.f, myColliderOffSetY, 0.f));
	cct->SetGravity(982.f);

	//myGameObject->SetLayers(eLayer::ENEMY | eLayer::ENEMY_DAMAGEABLE, eLayer::PLAYER, eLayer::DEFAULT);

	myGameObject->SetLayers(eLayer::ENEMY | eLayer::ENEMY_DAMAGEABLE | eLayer::IMMOVABLE, eLayer::DEFAULT, eLayer::NONE);
	myGameObject->SetTag(eTag::ENEMY);

	myAnimationController = &GetGameObject()->GetComponent<Engine::AnimatorComponent>()->GetController();
	Enemy::SetEnemyCondition(*myAnimationController, "InShootingRange", true);

	//myBlackboard = BT_Blackboard(GetGameObject(), myAnimationController);
	myBlackboard.SetGameObject(GetGameObject());
	myBlackboard.SetAnimationController(myAnimationController);
	//myShootBoneIndex = myBlackboard.GetAnimationController()->GetBoneIndexFromName("JackySocket");
	myShootBoneIndex = myBlackboard.GetAnimationController()->GetBoneIndexFromName("Eye");

	myBlackboard.setBool("EnterShootRange", true);
	myBlackboard.setBool("ExitShootRange", true);
	myBlackboard.setBool("IsDead", false);
	//myBlackboard.setFloat("ShootingRange", 1500.f);

		myBTree = BT_Builder()
			.composite<BT_Selector>()
				.composite<BT_Sequence>()
					.leaf<BTN_Mortar_InRangeOfPlayer>(&myBlackboard)
					.leaf<BTN_Mortar_Shoot>(&myBlackboard, this)
				.end()
				.composite<BT_Sequence>()
					.leaf<BTN_Mortar_OutRangeOfPlayer>(&myBlackboard)
					.leaf<BTN_MortarIdle>(&myBlackboard, this)
				.end()
			.end()
			.build();

	InitAudio();
	InitAnimationCallbacks();
}

void Mortar::Execute(Engine::eEngineOrder aOrder)
{
	EnemyBase::Execute(aOrder);// Mandatory to be able to work correctly, do not remove!

	Update(Time::DeltaTime); // Mandatory to be able to work correctly, do not remove!
}

void Mortar::Update(float aDeltaTime)
{
	if (myBlackboard.getBool("IsDead"))
		return;

	//myGravityTimer += Time::DeltaTime;
	//if (myGravityTimer > myGravityTime)
		//cct->SetGravity(0.f);

	{ // Update head collider pos follows
		Mat4f boneTrans = myAnimationController->GetBoneTransformWorld(myShootBoneIndex);
		Vec3f translation;
		Quatf rotation;
		Vec3f scale;
		boneTrans.Decompose(translation, rotation, scale);
		myBlackboard.setVec3f("ShootPosition", translation);

		GDebugDrawer->DrawSphere3D(
			DebugDrawFlags::AI,
			myBlackboard.getVec3f("ShootPosition"),
			50.f,
			0.f,
			{ 1.f, 0.f, 1.f, 1.f },
			false);
	}

	if (!myHasFinsihedSpawning)
		return;

	myBTree.update();

	GDebugDrawer->DrawSphere3D(
		DebugDrawFlags::AI,
		myGameObject->GetTransform().GetPosition(),
		myBlackboard.getFloat("ShootingRange"),
		0.f,
		{ 1.f, 0.f, 1.f, 1.f });
}

void Mortar::Reflect(Engine::Reflector& aReflector)
{
	EnemyBase::Reflect(aReflector);

	aReflector.Reflect(myBlackboard.GetMortarShootData(), "Shoot Data");
	aReflector.Reflect(myShootingRange, "Shoot Range");
	myBlackboard.setFloat("ShootingRange", myShootingRange);
	aReflector.Separator();
	aReflector.Reflect(myColliderHeight, "Collider Height");
	aReflector.Reflect(myColliderRadius, "Collider Radius");
	aReflector.Reflect(myColliderOffSetY, "Collider OffSet Y");
}

BT_Blackboard& Mortar::GetBlackboard()
{
	return myBlackboard;
}

void Mortar::InitAnimationCallbacks()
{
	//myBlackboard.GetAnimationController()->AddEventCallback(
	//	"RunFirstFrame",
	//	[this]()
	//	{
	//		myAudioComponent->PlayEvent("LabCoatRun"); // SoundImplementation LabCoatRun
	//	}
	//);


	/*myAnimationController->AddStateOnExitCallback(
		"Base Layer",
		"Aggro",
		[this]()
		{
			myBlackboard.setBool("AggroAnimOnExitCallback", true);
		}
	);*/

	myAnimationController->AddStateOnExitCallback(
		"BASE",
		"Spawn",
		[this]()
		{
			myHasFinsihedSpawning = true;
		}
	);

	myAnimationController->AddStateOnExitCallback(
		"BASE",
		"Reveal",
		[this]()
		{
			myAudioComponent->PlayEvent("Reveal");
			int hej = -1;
		}
	);
	myAnimationController->AddStateOnExitCallback(
		"BASE",
		"Retract",
		[this]()
		{
			myAudioComponent->PlayEvent("Retract");
			int hej = -1;
		}
	);

	{
		myBlackboard.GetAnimationController()->AddEventCallback(
			"ShootFrame",
			[this]()
			{
				Shoot(&myBlackboard);
				/*GDebugDrawer->DrawSphere3D(
					DebugDrawFlags::AI,
					myGameObject->GetTransform().GetPosition(),
					100.f,
					0.1f,
					{ 1.f, 0.f, 0.f, 1.f });*/
			}
		);
	}
}

void Mortar::Render()
{
	EnemyBase::Render();
	DebugDraw();
}

void Mortar::DebugDraw()
{

	/*GDebugDrawer->DrawSphere3D(
		DebugDrawFlags::AI,
		aggroPos,
		myBlackboard.getFloat("AggroRange"),
		0.f,
		{ 1.f, 0.f, 0.f, 1.f });*/
}

void Mortar::InitAudio()
{
	myAudioComponent = myGameObject->AddComponent<Engine::AudioComponent>();
	myAudioComponent->AddEvent("Idle", "event:/SFX/NPC/MORTAR/Idle");
	myAudioComponent->AddEvent("Hurt", "event:/SFX/NPC/MORTAR/Hurt");
	myAudioComponent->AddEvent("Mortar", "event:/SFX/NPC/MORTAR/MortarSpit");
	myAudioComponent->AddEvent("Death", "event:/SFX/NPC/MORTAR/Death");
	myAudioComponent->AddEvent("Retract", "event:/SFX/NPC/MORTAR/Retract");
	myAudioComponent->AddEvent("Reveal", "event:/SFX/NPC/MORTAR/Reveal");

	myBlackboard.SetAudioComponent(myAudioComponent);
}
void Mortar::OnDamage()
{
	GetBlackboard().GetAudioComponent()->PlayEvent("Hurt");
}

void Mortar::SpawnDeathSound()
{
}
