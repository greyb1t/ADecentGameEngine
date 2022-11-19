#include "pch.h"
#include "HandPunch_Buildup.h"
#include "AI/FinalBoss/Behaviours/HandPunch.h"
#include "AI/FinalBoss/LeftPunchHand.h"
#include "Engine/GameObject/GameObject.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "AI/FinalBoss/Constants.h"

FB::HandPunch_Buildup::HandPunch_Buildup(HandPunch& aParent)
	: myHandPunch(aParent)
{
}

void FB::HandPunch_Buildup::Update()
{
	myTimer.Tick(Time::DeltaTime);

	auto polling = myHandPunch.GetLeftHand().GetGameObject()->GetSingletonComponent<PollingStationComponent>();
	const auto playerPos = polling->GetPlayerPos();

	auto& modelTransform = myHandPunch.GetLeftHand().GetTransform();

	const Vec3f curvePos = myBezier.Evaluate(myTimer.Percent());
	modelTransform.SetPosition(curvePos);
	myHandPunch.GetLeftHand().SetTargetRotation(modelTransform.GetLookAtResult(playerPos));
}

bool FB::HandPunch_Buildup::IsFinished()
{
	return myTimer.IsFinished();
}

void FB::HandPunch_Buildup::OnEnter()
{
	myHandPunch.GetLeftHand().GetAnimator().GetController().Trigger(Constants::JabTriggerName);
	myHandPunch.GetLeftHand().GetAnimator().GetController().SetInt(Constants::JabStateIntName, 0);

	auto polling = myHandPunch.GetLeftHand().GetGameObject()->GetSingletonComponent<PollingStationComponent>();
	const auto playerPos = polling->GetPlayerPos();

	myTimer = TickTimer::FromSeconds(2.f * myHandPunch.GetLeftHand().GetEnragedDurationMultiplier());
	const Vec3f playerToHand = (myHandPunch.GetLeftHand().GetTransform().GetPosition() - playerPos).GetNormalized();
	const float buildupDistance = 500.f;
	myBezier = {};
	myBezier.AddPoint(myHandPunch.GetLeftHand().GetTransform().GetPosition());
	myBezier.AddPoint(myHandPunch.GetLeftHand().GetTransform().GetPosition() + playerToHand * buildupDistance);
}

void FB::HandPunch_Buildup::OnExit()
{
}
