#include "pch.h"
#include "HandPunch_Returning.h"
#include "AI/FinalBoss/Behaviours/HandPunch.h"
#include "AI/FinalBoss/LeftPunchHand.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "AI/FinalBoss/Constants.h"

FB::HandPunch_Returning::HandPunch_Returning(HandPunch& aParent)
	: myHandPunch(aParent)
{
	myHandPunch.GetLeftHand().GetAnimator().GetController().AddStateOnExitCallback(
		"Base", "Jab End", [this]() { myIsFinished = true; });
}

void FB::HandPunch_Returning::Update()
{
	/*
	myTimer.Tick(Time::DeltaTime);

	auto& modelTransform = myHandPunch.GetLeftHand().GetTransform();

	const Vec3f curvePos = myBezier.Evaluate(myTimer.Percent());
	const Vec3f nextCurvePos = myBezier.Evaluate(myTimer.Percent() + 0.001f);
	const Vec3f direction = (nextCurvePos - curvePos).GetNormalized();
	modelTransform.SetPosition(curvePos);
	modelTransform.LookAt(modelTransform.GetPosition() + direction);
	*/
}

bool FB::HandPunch_Returning::IsFinished()
{
	return myIsFinished;
	// return myTimer.IsFinished();
}

void FB::HandPunch_Returning::OnEnter()
{
	myHandPunch.GetLeftHand().GetAnimator().GetController().SetInt(Constants::JabStateIntName, 2);

	myIsFinished = false;

	// myTimer = TickTimer::FromSeconds(3.f);

	myBezier = {};
	myBezier.AddPoint(myHandPunch.GetLeftHand().GetTransform().GetPosition());
	myBezier.AddPoint(myHandPunch.GetInitialPosition());
}

void FB::HandPunch_Returning::OnExit()
{
}
