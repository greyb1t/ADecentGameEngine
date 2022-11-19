#include "pch.h"
#include "RightProjectileHand.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "Engine/GameObject/Components/RigidBodyComponent.h"
#include "Behaviours/HandHomingLaser.h"
#include "Behaviours/Behaviour.h"
#include "Behaviours/HandGattlingEye.h"
#include "Behaviours/RightHand_HandCrawler.h"
#include "Components/HealthComponent.h"
#include "Behaviours/HandIdle.h"
#include "Behaviours/HandDeath.h"
#include "AI/AIDirector/AIDirector.h"
#include "FinalBoss.h"
#include "LeftPunchHand.h"
#include "Behaviours/HandEnraged.h"
#include "Behaviours/HandIntro.h"

FB::RightProjectileHand::RightProjectileHand(GameObject* aGameObject)
	: FinalBossHand(aGameObject)
{
}

FB::RightProjectileHand::RightProjectileHand()
{
}

FB::RightProjectileHand::~RightProjectileHand()
{
}

void FB::RightProjectileHand::Awake()
{
	FinalBossHand::Awake();

	if (!myFinalBossGameObject)
	{
		LOG_ERROR(LogType::Game) << "Hand is missing reference to its model";
		myGameObject->Destroy();
		return;
	}

	myFinalBoss = myFinalBossGameObject.Get()->GetComponent<FinalBoss>();
}

void FB::RightProjectileHand::Start()
{
	FinalBossHand::Start();

	AddNormalBehaviour(RightHandBehaviours::Idle, MakeOwned<HandIdle>());
	AddNormalBehaviour(RightHandBehaviours::Death, MakeOwned<HandDeath>(*this, *myAnimator));
	AddNormalBehaviour(RightHandBehaviours::Enraged, MakeOwned<HandEnraged>(*this, *myAnimator));
	AddNormalBehaviour(RightHandBehaviours::Intro, MakeOwned<HandIntro>(*this));

	AddAttackBehaviour(RightHandBehaviours::HomingLaser, MakeOwned<HandHomingLaser>(*this));
	AddAttackBehaviour(RightHandBehaviours::GatlingEye, MakeOwned<HandGattlingEye>(*this));

	// TEMP TEMP TEMP
	// AddAttackBehaviour(LeftHandBehaviours::Crawler, MakeOwned<RightHand_HandCrawler>(*this));

	SetBaseBehaviour(RightHandBehaviours::Idle);

	SetTargetRotation(GetTransform().GetLookAtResult(myFinalBoss->GetBrokenBridgePos()));
}

void FB::RightProjectileHand::Execute(Engine::eEngineOrder aOrder)
{
	if (IsDemolished())
	{
		return;
	}

	FinalBossHand::Execute(aOrder);

	switch (myFinalBoss->GetPhase())
	{
	case FinalBossPhase::Phase1:
		// Add crawler when other hand is destroyed
		if (!GetAttackBehaviour(RightHandBehaviours::Crawler) &&
			myFinalBoss->GetLeftHand().IsDead())
		{
			AddAttackBehaviour(RightHandBehaviours::Crawler, MakeOwned<RightHand_HandCrawler>(*this));
		}

		if (myFinalBoss->GetLeftHand().IsDead())
		{
			Enrage(RightHandBehaviours::Enraged);
		}

		break;
	default:
		break;
	}
}

void FB::RightProjectileHand::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Header("Hand");

	FinalBossHand::Reflect(aReflector);

	aReflector.Separator();
	aReflector.Header("Right Hand");

	aReflector.Reflect(myFinalBossGameObject, "Final Boss");

	aReflector.Reflect(myHomingLaserDesc, "Homing Laser");
	aReflector.Reflect(myGattlingEyeDesc, "Gattling Eye");

	aReflector.Separator();

	RightHandBehaviours behaviour = RightHandBehaviours::_from_integral(myActiveBehaviourValue);
	if (aReflector.ReflectEnum(behaviour, "Behaviour"))
	{
		ChangeBehaviour(behaviour);
	}
}

const FB::HomingLaserDesc& FB::RightProjectileHand::GetHomingLaserDesc() const
{
	return myHomingLaserDesc;
}

const FB::GattlingEyeDesc& FB::RightProjectileHand::GetGattlingEyeDesc() const
{
	return myGattlingEyeDesc;
}

bool FB::RightProjectileHand::IsDemolished() const
{
	return GetNormalBehaviour(RightHandBehaviours::Death)->IsFinished();
}

void FB::RightProjectileHand::OnEnterPhase(const FinalBossPhase aPhase)
{
	switch (aPhase)
	{
	case FinalBossPhase::Introduction:
		ChangeBehaviour(RightHandBehaviours::Intro);
		break;
	case FinalBossPhase::Phase1:
		myHealth->SetIsInvincible(false);
		break;
	default:
		break;
	}
}

float FB::RightProjectileHand::GetMaxAllowedDistanceAbovePortal()
{
	return 0.f;
}

void FB::RightProjectileHand::OnDeath()
{
	ChangeBehaviour(RightHandBehaviours::Death);
}