#include "pch.h"
#include "Slime.h"

#include <cmath>
#include <math.h>

#include "AI/BehaviorTree/Base/BT_Builder.h"
#include "AI/BehaviorTree/Base/BT_Inverter.h"
#include "AI/BehaviorTree/Base/BT_Selector.h"
#include "AI/BehaviorTree/Base/BT_Sequence.h"
#include "AI/Enemies/Base/EnemyScalableStats.h"
#include "AI/GeneralNodes/BTN_FlyTowardsPlayer.h"
#include "AI/GeneralNodes/BTN_InRangeOfPlayer.h"
#include "AI/GeneralNodes/BTN_MoveTowardsPlayer.h"
#include "AI/Enemies/Projectiles/PoisonCloud.h"

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

Slime::Slime(GameObject* aGameObject) :
	EnemyBase(aGameObject)
{
}

void Slime::Start()
{
	EnemyBase::Start();

	myState = EnemyState::Neutral;

	SetEnemyShootType(eEnemyShootType::Laser);
	const Vec3f playerpos = myGameObject->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos();
	GDebugDrawer->DrawSphere3D(DebugDrawFlags::Always, playerpos, 100.f, 0.f, { 1,0,0,1 });

	const auto enemyScalableStatsComp = myGameObject->GetComponent<EnemyScalableStats>();
	myEnemyScalableStats = enemyScalableStatsComp->GetScalableValues();

	std::function<void()> deSpawnFunction = [this]()
	{
		myGameObject->Destroy();
	};
	SetDeSpawnFunction(deSpawnFunction);

	std::function<void()> deathFunction = [this]()
	{
		OnDeath();
	};

	auto health = myGameObject->AddComponent<EnemyHealth>(myEnemyScalableStats.myHealth);
	health->Init(&myBlackboard, deathFunction);

	cct = myGameObject->AddComponent<Engine::CharacterControllerComponent>();
	cct->Init(myColliderHeight, myColliderRadius);
	cct->SetMovementSettings(.01f);
	cct->SetOffset(Vec3f(0.f, myColliderOffSetY, 0.f));
	cct->SetGravity(myGravity * 2.f);

	myGameObject->SetLayers(eLayer::ENEMY | eLayer::ENEMY_DAMAGEABLE, eLayer::DEFAULT, eLayer::NONE);
	myGameObject->SetTag(eTag::ENEMY);

	myAnimationController = &GetGameObject()->GetComponent<Engine::AnimatorComponent>()->GetController();

	myBlackboard.SetGameObject(GetGameObject());
	myBlackboard.SetAnimationController(myAnimationController);

	/*myBTree = BT_Builder()
		.composite<BT_Selector>()
			.composite<BT_Sequence>()
				.decorator<BT_Inverter>()
					.leaf<BTN_InRangeOfPlayer>(&myBlackboard, myMeleeAttackRange)
				.end()
			.end()
		.end()
		.build();*/

	InitAudio();
	InitAnimationCallbacks();
}

void Slime::Execute(Engine::eEngineOrder aOrder)
{
	EnemyBase::Execute(aOrder);// Mandatory to be able to work correctly, do not remove!

	Update(Time::DeltaTime); // Mandatory to be able to work correctly, do not remove!
}
void Slime::OnDamage()
{
	myAudioComponent->PlayEvent("Hurt");
}

void Slime::SpawnDeathSound()
{
}

void Slime::Update(float aDeltaTime)
{
	if (myBlackboard.getBool("IsDead"))
	{
		if (GetGameObject()->GetComponent<Engine::CharacterControllerComponent>())
		{
			if (!myDeathFart && GetGameObject()->GetComponent<Engine::CharacterControllerComponent>()->IsGrounded())
			{
				SummonPoisonCloud(3.f);
				myDeathFart = true;
			}
		}
		return;
	}

	if (!myHasFinishedSpawning)
		return;

	CU::Vector3f pos;
	int h = 0;

	std::vector<OverlapHit> hits;

	switch (myState)
	{
	case Slime::EnemyState::Neutral:
		if (myTimer <= 0)
		{
			myStartPos = GetGameObject()->GetTransform().GetPosition();

			if (myBlackboard.GetAnimationController()->HasTrigger("Jump"))
			{
				myBlackboard.GetAnimationController()->Trigger("Jump");
			}

			myState = EnemyState::Jumping;
		}
		else
		{
			float lerpAm = 1 - myJumpRecharge / myTimer;
			//myGameObject->GetTransform().SetRotation(Quatf::Slerp(myGameObject->GetTransform().GetRotation(), Quatf(myJumpDirection), lerpAm));
			myTimer -= aDeltaTime;
		}
		break;
	case Slime::EnemyState::Jumping:

		if (GetGameObject()->GetComponent<Engine::CharacterControllerComponent>()->IsGrounded() && !myIsJumping)
		{
			GetGameObject()->GetComponent<Engine::CharacterControllerComponent>()->Jump(myJumpForce);
			myAudioComponent->PlayEvent("Jump");
			myIsJumping = true;
		}
		else if (!GetGameObject()->GetComponent<Engine::CharacterControllerComponent>()->IsGrounded() && myIsJumping)
		{
			GetGameObject()->GetComponent<Engine::CharacterControllerComponent>()->Move(myJumpDirection * myJumpSpeed, aDeltaTime);
		}
		else
		{
			FinnishJump();
		}
		break;
	case Slime::EnemyState::Landing:
		if (myTimer <= 0)
		{
			myTimer = myJumpRecharge;
			if (myBlackboard.GetAnimationController()->HasTrigger("Turn"))
			{
				myBlackboard.GetAnimationController()->Trigger("Turn");
			}
			SetRotation();

			myState = EnemyState::Neutral;
		}
		else
		{
			myTimer -= aDeltaTime;
		}
		break;
	default:
		break;
	}
}

void Slime::FinnishJump()
{
	myState = EnemyState::Landing;
	myTimer = myLandingTime;

	if (myBlackboard.GetAnimationController()->HasTrigger("Land"))
	{
		myBlackboard.GetAnimationController()->Trigger("Land");
		myBlackboard.GetAudioComponent()->PlayEvent("Land");
	}

	//Create new jumpdirection
	/*myJumpDirection = GetGameObject()->GetTransform().Forward();
	myJumpDirection.z = Random::RandomFloat(-1.f, 1.f);
	myJumpDirection.x = Random::RandomFloat(-1.f, 1.f);
	myJumpDirection.Normalize();*/

	myIsJumping = false;

	SummonPoisonCloud();
}

void Slime::SummonPoisonCloud(float aSizeModifier)
{
	GameObject* gameObject = myGameObject->GetScene()->AddGameObject<GameObject>();
	gameObject->GetTransform().SetPosition(GetGameObject()->GetTransform().GetPosition());
	gameObject->SetName("PoisonCloud");
	const auto& vfxSlamRef = gameObject->GetSingletonComponent<EnemyVFXRef>()->GetGroundTankSlam();
	mySlamVFX = gameObject->AddComponent<Engine::VFXComponent>(vfxSlamRef->Get());
	mySlamVFX->Play();

	const auto& vfxPoisonRef = gameObject->GetSingletonComponent<EnemyVFXRef>()->GetSlimePoison();
	myPoisonVFX = gameObject->AddComponent<Engine::VFXComponent>(vfxPoisonRef->Get());
	myPoisonVFX->GetTransform().SetScale({2, 1, 2});
	myPoisonVFX->Play();
	PoisonCloud* component = gameObject->AddComponent<PoisonCloud>();
	component->Init(this, myEnemyScalableStats.myDamage, myPoisonCloudDuration, myPoisonCloudRadius);
	myAudioComponent->PlayEvent("PoisonCloud");
}

void Slime::Reflect(Engine::Reflector& aReflector)
{
	EnemyBase::Reflect(aReflector);

	aReflector.Reflect(myColliderHeight, "Collider Height");
	aReflector.Reflect(myColliderRadius, "Collider Radius");
	aReflector.Reflect(myColliderOffSetY, "Collider OffSet Y");

	aReflector.Separator();

	aReflector.Reflect(myJumpRecharge, "Jump Cooldown");
	aReflector.Reflect(myJumpSpeed, "Jump Speed");
	aReflector.Reflect(myJumpForce, "Jump Force"); 
	aReflector.Reflect(myGravity, "Gravity");
	aReflector.Separator();
	aReflector.Reflect(myEnemyScalableStats.myDamage, "Poison Cloud DPS");
	aReflector.Reflect(myPoisonCloudDuration, "Poison Cloud Duration");
	aReflector.Reflect(myPoisonCloudRadius, "Poison Cloud Radius");
}

BT_Blackboard& Slime::GetBlackboard()
{
	return myBlackboard;
}

void Slime::InitAnimationCallbacks()
{
	{
		myAnimationController->AddStateOnExitCallback(
			"Base Layer",
			"Spawn",
			[this]()
			{
				myHasFinishedSpawning = true;
			}
		);
	}
}

void Slime::SetMyState(EnemyState aState)
{
	myState = aState;
}

void Slime::Render()
{
	EnemyBase::Render();
	DebugDraw();
}

void Slime::DebugDraw()
{


}

void Slime::InitAudio()
{
	myAudioComponent = myGameObject->AddComponent<Engine::AudioComponent>();
	myAudioComponent->AddEvent("Death", "event:/SFX/NPC/SLIME/Death");
	myAudioComponent->AddEvent("Hurt", "event:/SFX/NPC/SLIME/Hurt");
	myAudioComponent->AddEvent("Jump", "event:/SFX/NPC/SLIME/Jump");
	myAudioComponent->AddEvent("Land", "event:/SFX/NPC/SLIME/Land");
	myAudioComponent->AddEvent("PoisonCloud", "event:/SFX/NPC/SLIME/PoisonCloud");

	myBlackboard.SetAudioComponent(myAudioComponent);
}

void Slime::OnDeath()
{
	myBlackboard.setBool("IsDead", true);
	myGameObject->SetLayer(eLayer::NONE);
	//cct->Destroy(); // Jesper added this, any question ask him. It delete the cct on death
}

void Slime::SetRotation()
{
	const auto ownerPos = myBlackboard.GetOwner()->GetTransform().GetPosition();
	Vec3f targetPos;
	targetPos.x = ownerPos.x + Random::RandomFloat(-1000.f, 1000.f);
	targetPos.y = ownerPos.y;
	targetPos.z = ownerPos.z + Random::RandomFloat(-1000.f, 1000.f);

	myJumpDirection = targetPos - ownerPos;
	myJumpDirection.Normalize();

	float angle = atan2(myJumpDirection.z, myJumpDirection.x);
	SetTargetRotationRadians(angle + 90.f * Math::Deg2Rad, 6.f);
}