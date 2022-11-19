#include "pch.h"
#include "HandSlam_Search.h"
#include "AI/FinalBoss/Behaviours/HandSlam.h"
#include "AI/FinalBoss/LeftPunchHand.h"
#include "Engine/GameObject/GameObject.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "AI/FinalBoss/Constants.h"
#include "Engine/ResourceManagement/Resources/AnimationCurveResource.h"
#include "Engine/AnimationCurve/Curve.h"

FB::HandSlam_Search::HandSlam_Search(HandSlam& aParent)
	: myHandSlam(aParent)
{
	if (aParent.GetLeftHand().GetHandSlamDesc().mySearchSpeedCurve &&
		aParent.GetLeftHand().GetHandSlamDesc().mySearchSpeedCurve->IsValid())
	{
		mySpeedCurve = &aParent.GetLeftHand().GetHandSlamDesc().mySearchSpeedCurve->Get();
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Hand slam is missing search speed curve";
	}
}

void FB::HandSlam_Search::Update()
{
	myTimer.Tick(Time::DeltaTime);

	auto polling = myHandSlam.GetLeftHand().GetGameObject()->GetSingletonComponent<PollingStationComponent>();
	const auto playerPos = polling->GetPlayerPos();

	const float upDistance = myHandSlam.GetLeftHand().GetHandSlamDesc().mySearchDistanceAbovePlayer;

	myBezier = { };
	myBezier.AddPoint(myStartPos);
	myBezier.AddPoint(playerPos + Vec3f(0.f, 1.f, 0.f) * upDistance);

	auto& modelTransform = myHandSlam.GetLeftHand().GetTransform();

	float speedT = 1.f;

	if (mySpeedCurve)
	{
		speedT = mySpeedCurve->Evaluate(myTimer.Percent());
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Hand slam is missing search speed curve";
	}

	const float t = myTimer.Percent() * speedT;

	const Vec3f curvePos = myBezier.Evaluate(t);
	const Vec3f nextCurvePos = myBezier.Evaluate(t + 0.001f);
	const Vec3f direction = (nextCurvePos - curvePos).GetNormalized();
	modelTransform.SetPosition(curvePos);
	auto r = modelTransform.GetLookAtResult(modelTransform.GetPosition() + direction);
	myHandSlam.GetLeftHand().SetTargetRotation(r);

	PlaySound();
}

void FB::HandSlam_Search::PlaySound()
{
	if (!myPlayedSearchSound && myTimer.Percent() >= 0.55f)
	{
		myPlayedSearchSound = true;
		myHandSlam.HandSlamSound(true);
	}
}

bool FB::HandSlam_Search::IsFinished()
{
	return myTimer.IsFinished();
}

void FB::HandSlam_Search::OnEnter()
{
	myHandSlam.GetLeftHand().GetAnimator().GetController().SetInt(Constants::SlamStateIntName, 1);

	myTimer = TickTimer::FromSeconds(2.f * myHandSlam.GetLeftHand().GetEnragedDurationMultiplier());

	myStartPos = myHandSlam.GetLeftHand().GetTransform().GetPosition();

	myPlayedSearchSound = false;
}

void FB::HandSlam_Search::OnExit()
{
	myHandSlam.HandSlamSound(false);
}
