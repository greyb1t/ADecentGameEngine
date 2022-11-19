#include "pch.h"
#include "GlassCannon.h"

#include "Components/HealthComponent.h"

#include "AI/BehaviorTree/Base/BT_Builder.h"
#include "AI/BehaviorTree/Base/BT_Inverter.h"
#include "AI/BehaviorTree/Base/BT_Selector.h"
#include "AI/BehaviorTree/Base/BT_Sequence.h"
#include "AI/Enemies/Base/EnemyScalableStats.h"
#include "AI/GeneralNodes/BTN_FlyTowardsPlayer.h"
#include "AI/GeneralNodes/BTN_InRangeOfPlayer.h"
#include "AI/GeneralNodes/BTN_MoveTowardsPlayer.h"
#include "AI/Enemies/GlassCannon/BTN_FireLaser.h"

#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "Engine/GameObject/Components/AudioComponent.h"
#include "Engine/GameObject/Components/CharacterControllerComponent.h"
#include "Engine/Reflection/Reflector.h"
#include "AI\Enemies\Base\EnemyScalableStats.h"
#include "AI\Health\EnemyHealth.h"

GlassCannon::GlassCannon(GameObject* aGameObject) :
	EnemyBase(aGameObject)
{
}

void GlassCannon::Start()
{
	EnemyBase::Start();

	//myLaserBeam.Init(this, myEnemyScalableStats.myDamage, myLaserChargeTime, myLaserFireTime, myLaserCoolDown);

	SetEnemyShootType(eEnemyShootType::Laser);
	const Vec3f playerpos = myGameObject->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos();
	GDebugDrawer->DrawSphere3D(DebugDrawFlags::Always, playerpos, 100.f, 0.f, { 1,0,0,1 });

	const auto enemyScalableStatsComp = myGameObject->GetComponent<EnemyScalableStats>();
	myEnemyScalableStats = enemyScalableStatsComp->GetScalableValues();

	auto health = myGameObject->AddComponent<EnemyHealth>(myEnemyScalableStats.myHealth);
	health->Init(&myBlackboard);

	std::function<void(float)> stunFunction = [this](float aStunTime)
	{
		myStunned = true;
		myStunTimer = aStunTime;

		GetGameObject()->GetComponent<Engine::AnimatorComponent>()->SetActive(false);
	};

	SetUpStun(stunFunction);

	std::function<void(float, float)> slowFunction = [this](float aSlowPercentage, float aSlowTime)
	{
		mySlowed = true;

		//Slow Animation
		//GetGameObject()->GetComponent<Engine::AnimatorComponent>()->SetActive(false);
	};

	SetUpSlow(slowFunction);

	std::function<void(Vec3f)> knockBackFunction = [this](Vec3f aKnockDirection)
	{
		myStunned = true;

		myKnockBackDirection = aKnockDirection;
	};

	SetUpKnockBack(knockBackFunction);

	cct = myGameObject->AddComponent<Engine::CharacterControllerComponent>();
	cct->Init(90.f, 45.f);
	cct->SetMovementSettings(.01f);
	cct->SetOffset(Vec3f(0.f, 0.f, 0.f));
	cct->SetGravity(false);

	myGameObject->SetLayers(eLayer::ENEMY | eLayer::ENEMY_DAMAGEABLE, eLayer::DEFAULT, eLayer::DEFAULT);
	myGameObject->SetTag(eTag::ENEMY);

	myAnimationController = &GetGameObject()->GetComponent<Engine::AnimatorComponent>()->GetController();

	myBlackboard = BT_Blackboard(GetGameObject(), myAnimationController);

	myBTree = BT_Builder()
		.composite<BT_Selector>()
			.composite<BT_Sequence>()
				.leaf<BTN_InRangeOfPlayer>(&myBlackboard)
				.leaf<BTN_FireLaser>(&myBlackboard)
			.end()
			.composite<BT_Sequence>()
				.decorator<BT_Inverter>()
					.leaf<BTN_InRangeOfPlayer>(&myBlackboard)
				.end()
				.leaf<BTN_MoveTowardsPlayer>(&myBlackboard, this)
			.end()
		.end()
		.build();

	InitAudio();
	InitAnimationCallbacks();
}

void GlassCannon::Execute(Engine::eEngineOrder aOrder)
{
	EnemyBase::Execute(aOrder);// Mandatory to be able to work correctly, do not remove!

	Update(Time::DeltaTime); // Mandatory to be able to work correctly, do not remove!
}

void GlassCannon::Update(float aDeltaTime)
{
	myLaserBeam.Update(aDeltaTime);

	myBTree.update();
}

void GlassCannon::Reflect(Engine::Reflector& aReflector)
{
	EnemyBase::Reflect(aReflector);

	aReflector.Reflect(myEnemyScalableStats.myHealth, "Max Health");
	aReflector.Reflect(myEnemyScalableStats.myDamage, "Damage");
	aReflector.Reflect(myLaserChargeTime, "Laser Charge Up");
	aReflector.Reflect(myLaserFireTime, "Laser Fire Time");
	aReflector.Reflect(myLaserCoolDown, "Laser Cooldown");
}

BT_Blackboard& GlassCannon::GetBlackboard()
{
	return myBlackboard;
}

void GlassCannon::InitAnimationCallbacks()
{
	
}

void GlassCannon::IgniteLaser()
{
	myLaserBeam.Ignite();
}

LaserBeam::eFireState GlassCannon::CheckLaserStatus()
{
	return myLaserBeam.GetFireState();
}

void GlassCannon::Render()
{
	EnemyBase::Render();
	DebugDraw();
}

void GlassCannon::DebugDraw()
{


}

void GlassCannon::InitAudio()
{
	myAudioComponent = myGameObject->AddComponent<Engine::AudioComponent>();
	myAudioComponent->AddEvent("BeamAttack", "event:/SFX/NPC/GLASSCANNON/BeamAttack");
	myAudioComponent->AddEvent("BeamCharge", "event:/SFX/NPC/GLASSCANNON/BeamCharge");
	myAudioComponent->AddEvent("Death", "event:/SFX/NPC/GLASSCANNON/Death");
	myAudioComponent->AddEvent("Flying", "event:/SFX/NPC/GLASSCANNON/Flying");
	myAudioComponent->AddEvent("Hurt", "event:/SFX/NPC/GLASSCANNON/Hurt");

	myBlackboard.SetAudioComponent(myAudioComponent);
}