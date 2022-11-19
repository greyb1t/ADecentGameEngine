#include "pch.h"
#include "FlyPopcorn.h"

#include "AI/BehaviorTree/Base/BT_Builder.h"
#include "AI/BehaviorTree/Base/BT_Inverter.h"
#include "AI/BehaviorTree/Base/BT_Selector.h"
#include "AI/BehaviorTree/Base/BT_Sequence.h"
#include "AI/Enemies/Base/EnemyScalableStats.h"
#include "AI/Enemies/BurstTank/BTN_StayAboveGround.h"
#include "AI/GeneralNodes/BTN_FlyTowardsPlayer.h"
#include "AI/GeneralNodes/BTN_Fly_Offset_Wander.h"
#include "AI/GeneralNodes/BTN_InRangeOfPlayer.h"
#include "AI/GeneralNodes/BTN_MoveTowardsPlayer.h"
#include "AI/GeneralNodes/BTN_Shoot.h"
#include "AI/Health/EnemyHealth.h"
#include "Engine/Shortcuts.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "Engine/GameObject/Components/AudioComponent.h"
#include "Engine/GameObject/Components/CharacterControllerComponent.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"
#include "Engine/Reflection/Reflector.h"


FlyPopcorn::FlyPopcorn(GameObject* aGameObject) :
	EnemyBase(aGameObject)
{
}

//FlyPopcorn::~FlyPopcorn()
//{
//	myEyeGameObject->Destroy();
//}

void FlyPopcorn::Start()
{
	EnemyBase::Start();

	SetEnemyDeathType(EnemyDeathType::Explode);
	SetEnemyShootType(eEnemyShootType::RapidPopcorn);
	const Vec3f playerpos = myGameObject->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos();

	auto& scalableStats = myGameObject->GetComponent<EnemyScalableStats>()->GetScalableValues();

	std::function<void()> deSpawnFunction = [this]()
	{
		myEyeGameObject->Destroy();
		myGameObject->Destroy();
	};
	SetDeSpawnFunction(deSpawnFunction);

	std::function<void()> deathFunction = [this]()
	{
		GetBlackboard().GetAudioComponent()->PlayEvent("Hurt");
		SpawnDeathSound();
		cct->SetGravity(982.f);
		myBlackboard.setBool("IsDead", true);
		myEyeGameObject->Destroy();
	};
	myEnemyHealth = myGameObject->AddComponent<EnemyHealth>(scalableStats.myHealth);
	myEnemyHealth->Init(&myBlackboard, deathFunction);

	cct = myGameObject->AddComponent<Engine::CharacterControllerComponent>();
	cct->Init(myColliderHeight, myColliderRadius);
	cct->SetMovementSettings(.01f);
	cct->SetOffset(Vec3f(0.f, myColliderOffSetY, 0.f));
	cct->SetGravity(0.f);
	//cct->SetGravity(200.f);

	myGameObject->SetLayers(eLayer::ENEMY | eLayer::ENEMY_DAMAGEABLE, eLayer::DEFAULT, eLayer::DEFAULT);
	myGameObject->SetTag(eTag::ENEMY);

	myAnimationController = &GetGameObject()->GetComponent<Engine::AnimatorComponent>()->GetController();

	myBlackboard.setBool("IsDead", false);
	//myBlackboard.setFloat("ShootingRange", 1500.f);

	//myBlackboard = BT_Blackboard(GetGameObject(), myAnimationController);6

	myBlackboard.SetGameObject(GetGameObject());
	myBlackboard.SetAnimationController(myAnimationController);
	myShootBoneIndex = myBlackboard.GetAnimationController()->GetBoneIndexFromName("Eye");

	//myBlackboard.SetRapidShootData(&myRapidShootData);
	//myRapidShootData->myDamage = scalableStats.myDamage;
	//myRapidShootData->mySpeed = 300.0f;

	Engine::GameObjectPrefab& eye = myEye->Get();
	myEyeGameObject = &eye.Instantiate(*myGameObject->GetScene());
	//myEyeBoneIndex = myBlackboard.GetAnimationController()->GetBoneIndexFromName("Eye");
	myBlackboard.setFloat("Speed", 400.f);

	myBTree = BT_Builder()
		.composite<BT_Selector>()
		.composite<BT_Sequence>()
		.leaf<BTN_Fly_Offset_Wander>(&myBlackboard, this, 50.f)
		.leaf<BTN_InRangeOfPlayer>(&myBlackboard)
		.leaf<BTN_Shoot>(&myBlackboard, this)
		.end()
		.composite<BT_Sequence>()
		.decorator<BT_Inverter>()
		.leaf<BTN_InRangeOfPlayer>(&myBlackboard)
		.end()
		.leaf<BTN_MoveTowardsPlayer>(&myBlackboard, this)
		.leaf<BTN_Fly_Offset_Wander>(&myBlackboard, this, 50.f)
		.leaf<BTN_StayAboveGround>(&myBlackboard, this, myGroundCheckDistance)
		.end()
		.end()
		.build();

	InitAudio();
	InitAnimationCallbacks();
}

void FlyPopcorn::Execute(Engine::eEngineOrder aOrder)
{
	EnemyBase::Execute(aOrder);// Mandatory to be able to work correctly, do not remove!

	Update(Time::DeltaTime); // Mandatory to be able to work correctly, do not remove!
}

void FlyPopcorn::Update(float aDeltaTime)
{
	{ // Update head collider pos follows
		Mat4f boneTrans = myAnimationController->GetBoneTransformWorld(myShootBoneIndex);
		Vec3f translation;
		Quatf rotation;
		Vec3f scale;
		boneTrans.Decompose(translation, rotation, scale);
		myBlackboard.setVec3f("ShootPosition", translation);
		myEyeGameObject->GetTransform().SetPosition(translation);
	}

	if (myEyeGameObject)
	{
		myEyeGameObject->GetTransform().LookAt(myGameObject->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos());
	}

	if (myBlackboard.getBool("IsDead"))
		return;

	WingForce();

	myBTree.update();

	GDebugDrawer->DrawSphere3D(
		DebugDrawFlags::AI,
		myGameObject->GetTransform().GetPosition(),
		myBlackboard.getFloat("ShootingRange"),
		0.f,
		{ 1.f, 0.f, 1.f, 1.f });
}

void FlyPopcorn::Reflect(Engine::Reflector& aReflector)
{
	EnemyBase::Reflect(aReflector);
	//aReflector.Reflect(mySpeed, "Speed");

	aReflector.Reflect(myEye, "Eye Prefab");

	aReflector.Reflect(myBlackboard.GetRapidShootData(), "Shoot Data");
	aReflector.Reflect(myShootingRange, "Shoot Range");
	myBlackboard.setFloat("ShootingRange", myShootingRange);
	aReflector.Separator();
	aReflector.Reflect(myColliderHeight, "Collider Height");
	aReflector.Reflect(myColliderRadius, "Collider Radius");
	aReflector.Reflect(myColliderOffSetY, "Collider OffSet Y");
	aReflector.Separator();
	aReflector.Reflect(myGroundCheckDistance, "Ground Check Distance");
}

BT_Blackboard& FlyPopcorn::GetBlackboard()
{
	return myBlackboard;
}

void FlyPopcorn::InitAnimationCallbacks()
{
	myBlackboard.GetAnimationController()->AddEventCallback(
		"Up1",
		[this]()
		{
			myActivateWingForce = true;
		}
	);
	myBlackboard.GetAnimationController()->AddEventCallback(
		"Up2",
		[this]()
		{
			myActivateWingForce = true;
		}
	);
	myBlackboard.GetAnimationController()->AddEventCallback(
		"Down1",
		[this]()
		{
			myActivateWingForce = false;
			//cct->SetGravity(7.5f);
			/*	const Vec3f dir = Vec3f(0.f, -1.f, 0.f);
				const float speed = 200.f;
				const auto movement = dir * speed * Time::DeltaTime;
				myGameObject->GetTransform().Move(movement);*/
		}
	);
	myBlackboard.GetAnimationController()->AddEventCallback(
		"Down2",
		[this]()
		{
			myActivateWingForce = false;
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
}

void FlyPopcorn::Render()
{
	EnemyBase::Render();
	DebugDraw();
}

void FlyPopcorn::DebugDraw()
{
	if (myEyeGameObject)
	{
		GDebugDrawer->DrawLine3D(
			DebugDrawFlags::AI,
			myEyeGameObject->GetTransform().GetPosition(),
			myEyeGameObject->GetTransform().GetPosition() + myEyeGameObject->GetTransform().Forward().GetNormalized() * 100.f * -1.f,
			0.f,
			{ 1.f, 0.f, 1.f, 1.f });
	}


	/*GDebugDrawer->DrawSphere3D(
		DebugDrawFlags::AI,
		aggroPos,
		myBlackboard.getFloat("AggroRange"),
		0.f,
		{ 1.f, 0.f, 0.f, 1.f });*/
}

void FlyPopcorn::InitAudio()
{
	myAudioComponent = myGameObject->AddComponent<Engine::AudioComponent>();
	myBlackboard.SetAudioComponent(myAudioComponent);

	myAudioComponent->AddEvent("Explode", "event:/SFX/NPC/POPCORN/Explode");
	myAudioComponent->AddEvent("ProjectileExplode", "event:/SFX/NPC/ProjectileExplode");
	myAudioComponent->AddEvent("Flying", "event:/SFX/NPC/POPCORN/WingFlap");
	myAudioComponent->AddEvent("Projectile", "event:/SFX/NPC/POPCORN/Shoot");

	myAudioComponent->PlayEvent("Flying");
}
void FlyPopcorn::OnDamage()
{
	GetBlackboard().GetAudioComponent()->PlayEvent("Hurt");
}

void FlyPopcorn::SpawnDeathSound()
{
	auto gO = myGameObject->GetScene()->AddGameObject<GameObject>();
	gO->GetTransform().SetPosition(myGameObject->GetTransform().GetPosition());

	auto audio = gO->AddComponent<Engine::AudioComponent>();
	gO->AddComponent<Engine::DestructComponent>(5);
	audio->AddEvent("Explode", "event:/SFX/NPC/POPCORN/Explode");
	audio->PlayEvent("Explode");
}

void FlyPopcorn::WingForce()
{
	constexpr float speed = 25.f;
	if (myActivateWingForce)
	{
		const Vec3f dir = Vec3f(0.f, 1.f, 0.f);
		const auto movement = dir * speed * Time::DeltaTime;
		myGameObject->GetTransform().Move(movement);
	}
	else
	{
		const Vec3f dir = Vec3f(0.f, -1.f, 0.f);
		const auto movement = dir * speed * Time::DeltaTime;
		myGameObject->GetTransform().Move(movement);
	}

}
