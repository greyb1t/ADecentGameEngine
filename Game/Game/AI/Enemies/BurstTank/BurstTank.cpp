#include "pch.h"
#include "BurstTank.h"

#include "BTN_StayAboveGround.h"
#include "AI/BehaviorTree/Base/BT_Builder.h"
#include "AI/BehaviorTree/Base/BT_Inverter.h"
#include "AI/BehaviorTree/Base/BT_Selector.h"
#include "AI/BehaviorTree/Base/BT_Sequence.h"
#include "AI/Enemies/Base/EnemyScalableStats.h"
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
#include "Engine/Reflection/Reflector.h"

BurstTank::BurstTank(GameObject* aGameObject) :
	EnemyBase(aGameObject)
{
}

void BurstTank::Start()
{
	EnemyBase::Start();

	SetEnemyDeathType(EnemyDeathType::Explode);

	SetEnemyShootType(eEnemyShootType::BurstTank);

	auto& scalableStats = myGameObject->GetComponent<EnemyScalableStats>()->GetScalableValues();

	std::function<void()> deSpawnFunction = [this]()
	{
		myGameObject->Destroy();
	};
	SetDeSpawnFunction(deSpawnFunction);

	std::function<void()> deathFunction = [this]()
	{
		SpawnDeathSound();
		cct->SetGravity(982.f);
		myBlackboard.setBool("IsDead", true);
	};
	myEnemyHealth = myGameObject->AddComponent<EnemyHealth>(scalableStats.myHealth);
	myEnemyHealth->Init(&myBlackboard, deathFunction);

	cct = myGameObject->AddComponent<Engine::CharacterControllerComponent>();
	cct->Init(myColliderHeight, myColliderRadius);
	cct->SetMovementSettings(.01f);
	cct->SetOffset(Vec3f(0.f, myColliderOffSetY, 0.f));
	cct->SetGravity(0.f);

	myGameObject->SetLayers(eLayer::ENEMY | eLayer::ENEMY_DAMAGEABLE, eLayer::PLAYER, eLayer::DEFAULT);
	myGameObject->SetTag(eTag::ENEMY);

	myAnimationController = &GetGameObject()->GetComponent<Engine::AnimatorComponent>()->GetController();

	myBlackboard.SetGameObject(GetGameObject());
	myBlackboard.SetAnimationController(myAnimationController);
	//myShootBoneIndex = myBlackboard.GetAnimationController()->GetBoneIndexFromName("Thorax5");
	//myShootBoneIndex = myBlackboard.GetAnimationController()->GetBoneIndexFromName("JackySocket");
	myShootBoneIndex = myBlackboard.GetAnimationController()->GetBoneIndexFromName("ThoraxJaw");

	myBlackboard.setBool("IsDead", false);
	myBlackboard.setFloat("Speed", myMovementSpeed);
	//myBlackboard.setFloat("ShootingRange", 1500.f);
	//myBlackboard.setVec3f("ShootPosition", Vec3f(0.f, 0.f, 0.f));
	myBlackboard.setFloat("Speed", 400.f);


	myBTree = BT_Builder()
		.composite<BT_Selector>()
			.composite<BT_Sequence>()
				//.leaf<BTN_Fly_Offset_Wander>(&myBlackboard, this, 80.f)
				.leaf<BTN_InRangeOfPlayer>(&myBlackboard)
				.leaf<BTN_Shoot>(&myBlackboard, this)
			.end()
			.composite<BT_Sequence>()
				.decorator<BT_Inverter>()
					.leaf<BTN_InRangeOfPlayer>(&myBlackboard)
				.end()
				//.leaf<BTN_Fly_Offset_Wander>(&myBlackboard, this, 80.f)
				.leaf<BTN_MoveTowardsPlayer>(&myBlackboard, this)
				.leaf<BTN_StayAboveGround>(&myBlackboard, this, myGroundCheckDistance)
			.end()
		.end()
		.build();

	InitAudio();
	InitAnimationCallbacks();
}

void BurstTank::Execute(Engine::eEngineOrder aOrder)
{
	EnemyBase::Execute(aOrder);// Mandatory to be able to work correctly, do not remove!

	Update(Time::DeltaTime); // Mandatory to be able to work correctly, do not remove!
}

void BurstTank::Update(float aDeltaTime)
{
	//myGameObject->SetPhysicalLayer(eLayer::NONE);
	if (myBlackboard.getBool("IsDead"))
		return;

	{ // Update head collider pos follows
		Mat4f boneTrans = myAnimationController->GetBoneTransformWorld(myShootBoneIndex);
		Vec3f translation;
		Quatf rotation;
		Vec3f scale;
		boneTrans.Decompose(translation, rotation, scale);
		test = translation;
		myBlackboard.setVec3f("ShootPosition", translation);
		//LOG_INFO(LogType::AIAnimation) << translation.x << ", " << translation.y << ", " << translation.z;
	}

	myBTree.update();
	DebugDraw();
}

void BurstTank::Reflect(Engine::Reflector& aReflector)
{
	EnemyBase::Reflect(aReflector);
	aReflector.Reflect(myBlackboard.GetBurstShootData(), "Shoot Data");
	aReflector.Reflect(myShootingRange, "Shoot Range");
	myBlackboard.setFloat("ShootingRange", myShootingRange);

	aReflector.Reflect(myMovementSpeed, "Movement Speed");
	myBlackboard.setFloat("Speed", myMovementSpeed);
	aReflector.Separator();
	aReflector.Reflect(myColliderHeight, "Collider Height");
	aReflector.Reflect(myColliderRadius, "Collider Radius");
	aReflector.Reflect(myColliderOffSetY, "Collider OffSet Y");
	aReflector.Separator();
	aReflector.Reflect(myGroundCheckDistance, "Ground Check Distance");
}

BT_Blackboard& BurstTank::GetBlackboard()
{
	return myBlackboard;
}

void BurstTank::InitAnimationCallbacks()
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
}

void BurstTank::Render()
{
	EnemyBase::Render();
	
}

void BurstTank::DebugDraw()
{
	GDebugDrawer->DrawSphere3D(
		DebugDrawFlags::AI,
		test,
		50.f,
		0.f,
		{ 1.f, 0.f, 1.f, 1.f },
		false);
}

void BurstTank::InitAudio()
{
	myAudioComponent = myGameObject->AddComponent<Engine::AudioComponent>();
	myAudioComponent->AddEvent("Death", "event:/SFX/NPC/AIRTANK/Death");
	myAudioComponent->AddEvent("Flying", "event:/SFX/NPC/AIRTANK/Flying");
	myAudioComponent->AddEvent("Hurt", "event:/SFX/NPC/AIRTANK/Hurt");
	myAudioComponent->AddEvent("Orb", "event:/SFX/NPC/AIRTANK/Orb");

	myAudioComponent->PlayEvent("Flying");

	myBlackboard.SetAudioComponent(myAudioComponent);
}

void BurstTank::OnDamage()
{
	GetBlackboard().GetAudioComponent()->PlayEvent("Hurt");
}

void BurstTank::SpawnDeathSound()
{
	auto gO = myGameObject->GetScene()->AddGameObject<GameObject>();
	gO->GetTransform().SetPosition(myGameObject->GetTransform().GetPosition());

	auto audio = gO->AddComponent<Engine::AudioComponent>();
	gO->AddComponent<Engine::DestructComponent>(5);
	audio->AddEvent("Death", "event:/SFX/NPC/AIRTANK/Death");
	audio->PlayEvent("Death");
}
