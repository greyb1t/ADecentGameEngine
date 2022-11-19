#include "pch.h"
#include "LeftPunchHand.h"
#include "Behaviours/Behaviour.h"
#include "Behaviours/HandPunch.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "Engine/GameObject/Components/BezierSplineComponent.h"
#include "Engine/GameObject/Components/ModelComponent.h"
#include "Engine/GameObject/GameObjectRef.h"
#include "Behaviours/HandSlam.h"
#include "Engine/GameObject/Components/RigidBodyComponent.h"
#include "Components/HealthComponent.h"
#include "Behaviours/LeftHand_HandCrawler.h"
#include "Behaviours/HandIdle.h"
#include "Behaviours/HandDeath.h"
#include "FinalBoss.h"
#include "Game/AI/FinalBoss/RightProjectileHand.h"
#include "Behaviours/HandEnraged.h"
#include "Engine/ResourceManagement/Resources/AnimationCurveResource.h"
#include "Engine/AnimationCurve/Curve.h"
#include "Behaviours/HandIntro.h"

FB::LeftPunchHand::LeftPunchHand(GameObject* aGameObject)
	: FinalBossHand(aGameObject)
{
}

FB::LeftPunchHand::LeftPunchHand()
{
}

FB::LeftPunchHand::~LeftPunchHand()
{
}

GameObject* FB::LeftPunchHand::GetColliderGameObject()
{
	return myColliderGameObject.Get();
}

const FB::HandPunchDesc& FB::LeftPunchHand::GetHandPunchDesc() const
{
	return myHandPunchDesc;
}

const FB::HandSlamDesc& FB::LeftPunchHand::GetHandSlamDesc() const
{
	return myHandSlamDesc;
}

void FB::LeftPunchHand::OnEnterPhase(const FinalBossPhase aPhase)
{
	switch (aPhase)
	{
	case FinalBossPhase::Introduction:
		break;
	case FinalBossPhase::Phase1:
		myHealth->SetIsInvincible(false);
		break;
	default:
		break;
	}
}

bool FB::LeftPunchHand::IsDemolished() const
{
	return GetNormalBehaviour(LeftHandBehaviours::Death)->IsFinished();
}

float FB::LeftPunchHand::GetMaxAllowedDistanceAbovePortal()
{
	return std::max(myHandSlamDesc.mySearchDistanceAbovePlayer, 0.f);
}

void FB::LeftPunchHand::Awake()
{
	FinalBossHand::Awake();
}

void FB::LeftPunchHand::Start()
{
	FinalBossHand::Start();

	AddNormalBehaviour(LeftHandBehaviours::Idle, MakeOwned<HandIdle>());
	AddNormalBehaviour(LeftHandBehaviours::Death, MakeOwned<HandDeath>(*this, *myAnimator));
	AddNormalBehaviour(LeftHandBehaviours::Enraged, MakeOwned<HandEnraged>(*this, *myAnimator));

	AddAttackBehaviour(LeftHandBehaviours::HandPunch, MakeOwned<HandPunch>(*this));
	AddAttackBehaviour(LeftHandBehaviours::HandSlam, MakeOwned<HandSlam>(*this));

	// TEMP, remove this and add back below
	// AddAttackBehaviour(LeftHandBehaviours::Crawler, MakeOwned<LeftHand_HandCrawler>(*this));

	SetBaseBehaviour(LeftHandBehaviours::Idle);

	auto collider = myColliderGameObject.Get()->GetComponent<Engine::RigidBodyComponent>();

	collider->ObserveTriggerEnter([this](GameObject* g)
		{
			if (Behaviour* b = GetActiveBehaviour())
			{
				if (g)
				{
					b->OnCollisionEnter(*g);
				}
			}
		});

	collider->ObserveTriggerStay([this](GameObject* g)
		{
			if (Behaviour* b = GetActiveBehaviour())
			{
				if (g)
				{
					b->OnCollisionStay(*g);
				}
			}
		});

	myIntroBezierTimer = TickTimer::FromSeconds(2.f);
	myIntroBezierDelayTimer = TickTimer::FromSeconds(3.f);

	// Set its position to the start of the bezier in the beginning
	if (auto spline = myFinalBoss->GetLeftHandIntroBezier())
	{
		const Vec3f splinePos = spline->GetSpline().Evaluate(0.f);
		GetTransform().SetPosition(spline->GetGameObject()->GetTransform().GetPosition() + splinePos);
		SetTargetRotation(GetTransform().GetLookAtResult(myFinalBoss->GetBrokenBridgePos()));
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Missing left hand intro spline in FinalBoss";
	}
}

void FB::LeftPunchHand::Execute(Engine::eEngineOrder aOrder)
{
	if (IsDemolished())
	{
		return;
	}

	FinalBossHand::Execute(aOrder);

	switch (myFinalBoss->GetPhase())
	{
	case FinalBossPhase::Introduction:
	{
		myIntroBezierDelayTimer.Tick(Time::DeltaTime);

		if (myIntroBezierDelayTimer.IsFinished())
		{
			float t = 1.f;

			if (myIntroSplineSpeedCurve && myIntroSplineSpeedCurve->IsValid())
			{
				t = myIntroSplineSpeedCurve->Get().Evaluate(myIntroBezierTimer.Percent());
			}

			myIntroBezierTimer.Tick(Time::DeltaTime * t);

			if (auto spline = myFinalBoss->GetLeftHandIntroBezier())
			{
				const Vec3f splinePos = spline->GetSpline().Evaluate(myIntroBezierTimer.Percent());

				GetTransform().SetPosition(spline->GetGameObject()->GetTransform().GetPosition() + splinePos);
				SetTargetRotation(GetTransform().GetLookAtResult(myFinalBoss->GetBrokenBridgePos()));
			}
			else
			{
				LOG_ERROR(LogType::Game) << "Missing left hand intro spline in FinalBoss";
			}
		}
	} break;

	case FinalBossPhase::Phase1:
		// Add crawler when other hand is destroyed
		if (!GetAttackBehaviour(LeftHandBehaviours::Crawler) &&
			myFinalBoss->GetRightHand().IsDead())
		{
			AddAttackBehaviour(LeftHandBehaviours::Crawler, MakeOwned<LeftHand_HandCrawler>(*this));
		}

		if (myFinalBoss->GetRightHand().IsDead())
		{
			Enrage(RightHandBehaviours::Enraged);
		}

		break;
	default:
		break;
	}
}

void FB::LeftPunchHand::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Header("Hand");

	FinalBossHand::Reflect(aReflector);

	aReflector.Separator();
	aReflector.Header("Left Hand");

	aReflector.Reflect(myHandPunchDesc, "Hand Punch");
	aReflector.Reflect(myHandSlamDesc, "Hand Slam");
	aReflector.Reflect(myIntroSplineSpeedCurve, "Intro Spline Speed Curve");

	aReflector.Separator();

	LeftHandBehaviours behaviour = LeftHandBehaviours::_from_integral(myActiveBehaviourValue);
	if (aReflector.ReflectEnum(behaviour, "Behaviour"))
	{
		ChangeBehaviour(behaviour);
	}
}

void FB::LeftPunchHand::OnDeath()
{
	ChangeBehaviour(LeftHandBehaviours::Death);
}