#include "pch.h"
#include "GroundPopcorn.h"

#include "BTN_GroundPopcorn_Shoot.h"
#include "BTN_StupidWander.h"
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
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"
#include "Engine/VFX/VFXUtilities.h"
#include "Engine\GameObject/Components\RigidBodyComponent.h"

GroundPopcorn::GroundPopcorn(GameObject* aGameObject) :
	EnemyBase(aGameObject)
{
}

void GroundPopcorn::Start()
{
	EnemyBase::Start();


	SetEnemyShootType(eEnemyShootType::GroundPopcorn);
	const Vec3f playerpos = myGameObject->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos();

	auto& scalableStats = myGameObject->GetComponent<EnemyScalableStats>()->GetScalableValues();

	std::function<void()> deSpawnFunction = [this]()
	{
		myEyeGameObject->Destroy();
		myGameObject->Destroy();
	};
	SetDeSpawnFunction(deSpawnFunction);

	auto deathFunction = [&]()
	{
		myBlackboard.GetAudioComponent()->PlayEvent("Death");
		myBlackboard.GetAudioComponent()->StopIdle();
		cct->SetGravity(982.f);
		myBlackboard.setBool("IsDead", true);
		cct->Destroy();
	};

	myEnemyHealth = myGameObject->AddComponent<EnemyHealth>(scalableStats.myHealth);
	myEnemyHealth->Init(&myBlackboard, deathFunction);
	myPreviousHealth = myEnemyHealth->GetHealthComponent()->GetMaxHealth();

	std::function<void(float)> stunFunction = [this](float aStunTime)
	{
		if (SetStunValues(aStunTime))
		{
			if (GetGameObject()->GetComponent<Engine::AnimatorComponent>())
			{
				GetGameObject()->GetComponent<Engine::AnimatorComponent>()->SetActive(false);
			}
		}
	};

	SetUpStun(stunFunction);

	std::function<void(float, float)> slowFunction = [this](float aSlowPercentage, float aSlowTime)
	{
		mySlowed = true;

		//Slow Animation
		if (GetGameObject()->GetComponent<Engine::AnimatorComponent>())
		{
			GetGameObject()->GetComponent<Engine::AnimatorComponent>();
		}
	};

	SetUpSlow(slowFunction);

	std::function<void(Vec3f)> knockBackFunction = [this](Vec3f aKnockBackDirection)
	{
		if (SetKnockBackValues(aKnockBackDirection))
		{

		}
	};

	SetUpKnockBack(knockBackFunction);


	cct = myGameObject->AddComponent<Engine::CharacterControllerComponent>();
	cct->Init(myColliderHeight, myColliderRadius);
	cct->SetMovementSettings(.01f);
	cct->SetOffset(Vec3f(0.f, myColliderOffSetY, 0.f));
	cct->SetGravity(982.f);

	myGameObject->SetLayers(eLayer::ENEMY | eLayer::ENEMY_DAMAGEABLE, eLayer::DEFAULT, eLayer::DEFAULT);
	myGameObject->SetTag(eTag::ENEMY);

	myAnimationController = &GetGameObject()->GetComponent<Engine::AnimatorComponent>()->GetController();

	myBlackboard.SetGameObject(GetGameObject());
	myBlackboard.SetAnimationController(myAnimationController);
	myBlackboard.setFloat("Speed", myMovementSpeed);

	myShootBoneIndex = myBlackboard.GetAnimationController()->GetBoneIndexFromName("Eye");
	Engine::GameObjectPrefab& eye = myEye->Get();
	myEyeGameObject = &eye.Instantiate(*myGameObject->GetScene());
	myEnemyHealth->SetEyeGameObject(myEyeGameObject);

	myBTree = BT_Builder()
		.composite<BT_Selector>()
		.composite<BT_Sequence>()
		.leaf<BTN_InRangeOfPlayer>(&myBlackboard)
		.leaf<BTN_StupidWander>(&myBlackboard, this)
		.leaf<BTN_GroundPopcorn_Shoot>(&myBlackboard, this)
		.end()
		.composite<BT_Sequence>()
		.decorator<BT_Inverter>()
		.leaf<BTN_InRangeOfPlayer>(&myBlackboard)
		.end()
		.leaf<BTN_StupidWander>(&myBlackboard, this)
		.end()
		.end()
		.build();

	InitAudio();
	InitAnimationCallbacks();
}

void GroundPopcorn::Execute(Engine::eEngineOrder aOrder)
{
	EnemyBase::Execute(aOrder);// Mandatory to be able to work correctly, do not remove!

	Update(Time::DeltaTime); // Mandatory to be able to work correctly, do not remove!

	myAnimationController->SetFloat("Movement", 2.f);
}

void GroundPopcorn::Update(float aDeltaTime)
{
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

	if (myPreviousHealth > myEnemyHealth->GetHealthComponent()->GetHealth())
	{
		OnDamage();
	}
	myPreviousHealth = myEnemyHealth->GetHealthComponent()->GetHealth();

	if (myEyeGameObject)
	{
		myEyeGameObject->GetTransform().SetPosition(myBlackboard.getVec3f("ShootPosition"));

		myEyeGameObject->GetTransform().LookAt(myGameObject->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos());
	}

	if (myBlackboard.getBool("IsDead"))
		return;

	if (!myHasFinsihedSpawning)
		return;

	myBTree.update();
}

void GroundPopcorn::Reflect(Engine::Reflector& aReflector)
{
	EnemyBase::Reflect(aReflector);

	aReflector.Reflect(myEye, "Eye Prefab");
	aReflector.Reflect(myBlackboard.GetGroundPopcornShootData(), "Shoot Data");

	aReflector.Reflect(myMovementSpeed, "Movement Speed");
	myBlackboard.setFloat("Speed", myMovementSpeed);

	aReflector.Reflect(myShootingRange, "Shoot Range");
	myBlackboard.setFloat("ShootingRange", myShootingRange);

	aReflector.Reflect(myColliderHeight, "Collider Height");
	aReflector.Reflect(myColliderRadius, "Collider Radius");
	aReflector.Reflect(myColliderOffSetY, "Collider OffSet Y");

	aReflector.Reflect(myMoveSpeed, "Move Speed");
	myBlackboard.setFloat("MoveSpeed", myMoveSpeed);
}

void GroundPopcorn::OnDamage()
{
	GetBlackboard().GetAudioComponent()->PlayEvent("Hurt");
}

BT_Blackboard& GroundPopcorn::GetBlackboard()
{
	return myBlackboard;
}

void GroundPopcorn::InitAnimationCallbacks()
{
	myBlackboard.GetAnimationController()->AddEventCallback(
		"Footstep_PopcornGround",
		[this]()
		{
			myAudioComponent->PlayEvent("Run"); // SoundImplementation LabCoatRun
		}
	);


	myAnimationController->AddStateOnExitCallback(
		"Base Layer",
		"Spawn",
		[this]()
		{
			myHasFinsihedSpawning = true;
		}
	);
}

void GroundPopcorn::Render()
{
	EnemyBase::Render();
	DebugDraw();
}

void GroundPopcorn::DebugDraw()
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

void GroundPopcorn::InitAudio()
{
	myAudioComponent = myGameObject->AddComponent<Engine::AudioComponent>();
	myAudioComponent->AddEvent("Spawn", "event:/SFX/NPC/GROUNDCORN/Spawn");
	myAudioComponent->AddEvent("Death", "event:/SFX/NPC/GROUNDCORN/Death");
	myAudioComponent->AddEvent("Idle", "event:/SFX/NPC/GROUNDCORN/Idle");
	myAudioComponent->AddEvent("Hurt", "event:/SFX/NPC/GROUNDCORN/Hurt");
	myAudioComponent->AddEvent("Projectile", "event:/SFX/NPC/GROUNDCORN/Projectile");
	myAudioComponent->AddEvent("Run", "event:/FOL/NPC/Light");

	myAudioComponent->SetIdleSoundTimes(2, 5);
	myAudioComponent->SetIdleSoundPath("event:/SFX/NPC/GROUNDCORN/Idle");

	myBlackboard.SetAudioComponent(myAudioComponent);
}

void GroundPopcorn::SpawnDeathSound()
{
}
