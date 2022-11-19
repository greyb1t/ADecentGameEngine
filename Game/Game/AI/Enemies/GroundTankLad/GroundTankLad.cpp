#include "pch.h"
#include "GroundTankLad.h"

#include "AI/AIDirector/AIDirector.h"
#include "AI/BehaviorTree/Base/BT_Builder.h"
#include "AI/BehaviorTree/Base/BT_Inverter.h"
#include "AI/BehaviorTree/Base/BT_Selector.h"
#include "AI/BehaviorTree/Base/BT_Sequence.h"
#include "AI/Enemies/Base/EnemyScalableStats.h"
#include "AI/GeneralNodes/BTN_FlyTowardsPlayer.h"
#include "AI/GeneralNodes/BTN_InRangeOfPlayer.h"
#include "AI/GeneralNodes/BTN_MoveTowardsPlayer.h"
#include "AI/Enemies/GroundTankLad/BTN_TankFireLaser.h"
#include "AI/Enemies/GroundTankLad/BTN_GroundPound.h"
#include "AI/Enemies/GroundTankLad/BTN_InRangeOfMelee.h"

#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "Engine/GameObject/Components/AudioComponent.h"
#include "Engine/GameObject/Components/CharacterControllerComponent.h"
#include "Engine/Reflection/Reflector.h"
#include "AI\Enemies\Base\EnemyScalableStats.h"
#include "AI\Health\EnemyHealth.h"
#include "AI/VFX/EnemyVFXRef.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"
#include "Engine/GameObject/Components/AudioComponent.h"
#include "Engine/GameObject/Components/CameraShakeComponent.h"
#include "Engine/GameObject/Components/ModelComponent.h"


GroundTankLad::GroundTankLad(GameObject* aGameObject) :
	EnemyBase(aGameObject)
{
}

void GroundTankLad::Start()
{
	EnemyBase::Start();

	myState = EnemyState::Neutral;

	//Engine::GameObjectPrefab& indicator = myGameObject->GetSingletonComponent<AIDirector>()->GetLaserPrefab()->Get();
	//myLaserGo = &indicator.Instantiate(*myGameObject->GetScene());



	SetEnemyShootType(eEnemyShootType::Laser);
	const Vec3f playerpos = myGameObject->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos();
	GDebugDrawer->DrawSphere3D(DebugDrawFlags::Always, playerpos, 100.f, 0.f, { 1,0,0,1 });

	const auto enemyScalableStatsComp = myGameObject->GetComponent<EnemyScalableStats>();
	myEnemyScalableStats = enemyScalableStatsComp->GetScalableValues();

	std::function<void()> deSpawnFunction = [this]()
	{

		if (!mySafeDespawn)
		{
			if (myLaserGo)
				myLaserGo->Destroy();

			if (myEyeGameObject)
				myEyeGameObject->Destroy();

			if (myEyeEffectGameObject)
				myEyeEffectGameObject->Destroy();

			if (myGameObject)
				myGameObject->Destroy();
		}
	};
	SetDeSpawnFunction(deSpawnFunction);

	std::function<void()> deathFunction = [this]()
	{
		mySafeDespawn = true;

		myLaserGo->GetComponent<LaserTest>()->StopVFX();

		if (myLaserGo)
			myLaserGo->Destroy();

		myBlackboard.GetAudioComponent()->PlayEvent("Death");
		myBlackboard.setBool("IsDead", true);
		cct->Destroy(); // Jesper added this, any question ask him. It delete the cct on death
		//myGameObject->SetLayer(eLayer::NONE);
	};

	auto health = myGameObject->AddComponent<EnemyHealth>(myEnemyScalableStats.myHealth);
	health->Init(&myBlackboard, deathFunction);

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
	cct->SetGravity(982);

	myGameObject->SetLayers(eLayer::ENEMY | eLayer::ENEMY_DAMAGEABLE, eLayer::DEFAULT, eLayer::DEFAULT);
	myGameObject->SetTag(eTag::ENEMY);

	myAnimationController = &GetGameObject()->GetComponent<Engine::AnimatorComponent>()->GetController();
	myBlackboard = BT_Blackboard(GetGameObject(), myAnimationController);

	myLaserGo = myGameObject->GetScene()->AddGameObject<GameObject>();
	myLaserGo->AddComponent<LaserTest>(myGameObject->GetSingletonComponent<AIDirector>()->GetLaserPrefab(),
		myGameObject->GetSingletonComponent<AIDirector>()->GetLaserSightPrefab(), &myBlackboard, myLaserFireTime, myLaserChargeTime);

	myLaserBeam.Init(this, &myBlackboard, myEnemyScalableStats.myDamage, myLaserChargeTime, myLaserFireTime, myLaserCoolDown);
	myGroundPound.Init(this, myEnemyScalableStats.myDamage, myPoundChargeTime, myPoundCooldown);

	myShootBoneIndex = myBlackboard.GetAnimationController()->GetBoneIndexFromName("Eye");
	myPoundBoneIndex = myBlackboard.GetAnimationController()->GetBoneIndexFromName("LeftHand");
	Engine::GameObjectPrefab& eye = myEye->Get();
	Engine::GameObjectPrefab& eyeEffect = myEyeEffect->Get();
	myEyeGameObject = &eye.Instantiate(*myGameObject->GetScene());
	myEyeEffectGameObject = &eyeEffect.Instantiate(*myGameObject->GetScene());
	health->SetEyeGameObject(myEyeGameObject);
	health->SetEyeEffectGameObject(myEyeEffectGameObject);


	myBlackboard.setFloat("ShootingRange", myLaserAttackRange);
	myBlackboard.setFloat("MeleeRange", myMeleeAttackRange);
	myBlackboard.setInt("DelayValue", myTargetDelay);
	myBlackboard.setFloat("Damage", myEnemyScalableStats.myDamage);
	myBlackboard.setFloat("Speed", myMovementSpeed);
	myBlackboard.setFloat("PoundColliderSize", myPoundColliderSize);
	myBlackboard.setFloat("PoundOffset", myPoundPositionOffsetY);
	myBlackboard.setVec3f("EyeColor", myEyeColor);

	myBTree = BT_Builder()
		.composite<BT_Selector>()
		.composite<BT_Selector>()
		.composite<BT_Sequence>()
		.leaf<BTN_InRangeOfMelee>(&myBlackboard)
		.leaf<BTN_GroundPound>(&myBlackboard, this)
		.end()
		.composite<BT_Sequence>()
		.leaf<BTN_InRangeOfPlayer>(&myBlackboard)
		.composite<BT_Sequence>()
		.leaf<BTN_TankFireLaser>(&myBlackboard, myLaserGo, myLaserCoolDown, myStartLaserCoolDown)
		.leaf<BTN_MoveTowardsPlayer>(&myBlackboard, this)
		.end()
		.end()
		.composite<BT_Sequence>()
		.decorator<BT_Inverter>()
		.leaf<BTN_InRangeOfPlayer>(&myBlackboard)
		.end()
		.leaf<BTN_MoveTowardsPlayer>(&myBlackboard, this)
		.end()
		.end()
		.end()
		.build();

	InitAudio();
	InitAnimationCallbacks();
}

void GroundTankLad::Execute(Engine::eEngineOrder aOrder)
{
	EnemyBase::Execute(aOrder);// Mandatory to be able to work correctly, do not remove!

	Update(Time::DeltaTime); // Mandatory to be able to work correctly, do not remove!
}

void GroundTankLad::Update(float aDeltaTime)
{
	{ // Update hand collider pos follows
		Mat4f boneTrans = myAnimationController->GetBoneTransformWorld(myPoundBoneIndex);
		Vec3f translation;
		Quatf rotation;
		Vec3f scale;
		boneTrans.Decompose(translation, rotation, scale);
		myBlackboard.setVec3f("PoundPosition", translation);

		GDebugDrawer->DrawSphere3D(
			DebugDrawFlags::AI,
			translation,
			50.f,
			0.f,
			{ 1.f, 0.f, 1.f, 1.f },
			false);
	}

	if (myEyeGameObject)
	{
		{
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

		myEyeGameObject->GetTransform().SetPosition(myBlackboard.getVec3f("ShootPosition"));
		myEyeEffectGameObject->GetTransform().SetPosition(myBlackboard.getVec3f("ShootPosition"));

		myEyeGameObject->GetTransform().LookAt(myGameObject->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos());
		myEyeEffectGameObject->GetTransform().LookAt(myGameObject->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos());

		Vec3f color = myBlackboard.getVec3f("EyeColor");
		myEyeEffectGameObject->GetComponent<Engine::ModelComponent>()->GetMeshInstances()[0].GetMaterialInstance().SetFloat4("myColor", { color.x, color.y, color.z, 0 });
	}


	if (myBlackboard.getBool("IsDead"))
		return;

	if (!myHasFinishedSpawning)
		return;

	if (myLaserGo->GetComponent<LaserTest>()->GetLaserGo())
		myLaserGo->GetComponent<LaserTest>()->GetLaserGo()->GetTransform().SetPosition(myBlackboard.getVec3f("ShootPosition"));

	if (myLaserGo->GetComponent<LaserTest>()->GetLaserSightGo())
		myLaserGo->GetComponent<LaserTest>()->GetLaserSightGo()->GetTransform().SetPosition(myBlackboard.getVec3f("ShootPosition"));


	if (!myStunned && !myKnockedBack)
	{
		switch (myState)
		{
		case GroundTankLad::EnemyState::Neutral:
			myBTree.update();
			break;
		case GroundTankLad::EnemyState::Pounding:
			myGroundPound.Update(aDeltaTime);
			break;
		case GroundTankLad::EnemyState::FiringLaser:
			//myLaserBeam.Update(aDeltaTime);
			myBTree.update();
			break;
		default:
			break;
		}
	}
}

void GroundTankLad::Reflect(Engine::Reflector& aReflector)
{
	EnemyBase::Reflect(aReflector);
	aReflector.Reflect(myEye, "Eye Prefab");
	aReflector.Reflect(myEyeEffect, "Eye Effect Prefab");

	aReflector.Separator();

	aReflector.Reflect(myMovementSpeed, "Speed");
	myBlackboard.setFloat("Speed", myMovementSpeed);
	aReflector.Separator();

	aReflector.Reflect(myColliderHeight, "Collider Height");
	aReflector.Reflect(myColliderRadius, "Collider Radius");
	aReflector.Reflect(myColliderOffSetY, "Collider OffSet Y");
	aReflector.Separator();


	aReflector.Reflect(myLaserAttackRange, "Shooting Range");
	myBlackboard.setFloat("ShootingRange", myLaserAttackRange);

	aReflector.Reflect(myLaserChargeTime, "Laser Charge Up");
	aReflector.Reflect(myLaserFireTime, "Laser Fire Time");
	aReflector.Reflect(myLaserCoolDown, "Laser Cooldown");
	aReflector.Reflect(myStartLaserCoolDown, "Start Laser CoolDown");

	aReflector.Separator();
	aReflector.Reflect(myMeleeAttackRange, "Melee Range");
	myBlackboard.setFloat("MeleeRange", myMeleeAttackRange);
	aReflector.Reflect(myPoundChargeTime, "Pound Charge Time");
	aReflector.Reflect(myPoundCooldown, "Pound Cooldown");
	aReflector.Reflect(myPoundColliderSize, "Pound Collider Size");
	myBlackboard.setFloat("PoundColliderSize", myPoundColliderSize);
	aReflector.Reflect(myPoundPositionOffsetY, "Pound Offset");
	myBlackboard.setFloat("PoundOffset", myPoundPositionOffsetY);
}

BT_Blackboard& GroundTankLad::GetBlackboard()
{
	return myBlackboard;
}

void GroundTankLad::InitAnimationCallbacks()
{
	{
		myBlackboard.GetAnimationController()->AddEventCallback(
			"TriggerCollision",
			[this]()
			{
				myGroundPound.Attack(&myBlackboard);
				/*GDebugDrawer->DrawSphere3D(
					DebugDrawFlags::AI,
					myGameObject->GetTransform().GetPosition(),
					100.f,
					0.1f,
					{ 1.f, 0.f, 0.f, 1.f });*/

				const float distance = (myGameObject->GetScene()->GetMainCamera().GetTransform().GetPosition() -
					myGameObject->GetTransform().GetPosition()).Length();

				const auto mainCam = myGameObject->GetScene()->GetMainCameraGameObject();
				if (const auto shakeComponent = mainCam->GetComponent<Engine::CameraShakeComponent>())
				{
					shakeComponent->AddPerlinShakeByDistance("Enemy Ground Slam", 1500.f, distance);
				}
			}
		);
	}

	{
		myBlackboard.GetAnimationController()->AddStateOnExitCallback(
			"BASE",
			"Melee",
			[this]()
			{
				myGroundPound.EndAttack();
			}
		);
	}

	{
		myAnimationController->AddStateOnExitCallback(
			"BASE",
			"Spawn",
			[this]()
			{
				myHasFinishedSpawning = true;
			}
		);
	}
	myBlackboard.GetAnimationController()->AddEventCallback(
		"Footstep_Tankground",
		[this]()
		{
			myAudioComponent->PlayEvent("Run"); // SoundImplementation LabCoatRun
		}
	);
}

void GroundTankLad::IgniteLaser()
{
	myState = EnemyState::FiringLaser;
	myLaserBeam.Ignite();
}

void GroundTankLad::StartPounding()
{
	myState = EnemyState::Pounding;
	myGroundPound.InitiateAttack();

	myAudioComponent->PlayEvent("GroundSlam");
}

const LaserBeam::eFireState GroundTankLad::CheckLaserStatus() const
{
	return myLaserBeam.GetFireState();
}

const GroundPound::eFireState GroundTankLad::CheckPoundStatus() const
{
	return myGroundPound.GetFireState();
}

void GroundTankLad::SetMyState(EnemyState aState)
{
	myState = aState;
}

void GroundTankLad::Render()
{
	EnemyBase::Render();
	DebugDraw();
}

void GroundTankLad::DebugDraw()
{


}

void GroundTankLad::InitAudio()
{
	myAudioComponent = myGameObject->AddComponent<Engine::AudioComponent>();
	myAudioComponent->AddEvent("Death", "event:/SFX/NPC/GROUNDTANK/Death");
	myAudioComponent->AddEvent("GroundSlam", "event:/SFX/NPC/GROUNDTANK/GroundSlam");
	myAudioComponent->AddEvent("Hurt", "event:/SFX/NPC/GROUNDTANK/Hurt");
	myAudioComponent->AddEvent("LaserAttack", "event:/SFX/NPC/GROUNDTANK/LaserAttack");
	myAudioComponent->AddEvent("LaserCharge", "event:/SFX/NPC/GROUNDTANK/LaserCharge");
	myAudioComponent->AddEvent("Run", "event:/FOL/NPC/Heavy");

	myBlackboard.SetAudioComponent(myAudioComponent);

	myBlackboard.GetAnimationController()->AddEventCallback(
		"Footstep_TankGround",
		[this]()
		{
			myAudioComponent->PlayEvent("Run"); // SoundImplementation LabCoatRun
		}
	);
}

void GroundTankLad::OnDeath()
{
	myAudioComponent->PlayEvent("Death");
}

void GroundTankLad::SpawnDeathSound()
{
}
