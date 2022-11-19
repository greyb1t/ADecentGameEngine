#include "pch.h"
#include "HandSlam_End.h"
#include "AI/FinalBoss/Behaviours/HandSlam.h"
#include "AI/FinalBoss/LeftPunchHand.h"
#include "Engine/GameObject/GameObject.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "AI/FinalBoss/Constants.h"

FB::HandSlam_End::HandSlam_End(HandSlam& aParent)
	: myHandSlam(aParent)
{
	myHandSlam.GetLeftHand().GetAnimator().GetController().AddStateOnExitCallback("Base", "Slam End", 
		[this]()
		{ 
			myIsFinished = true;
		});
}

void FB::HandSlam_End::Update()
{
	/*
	myTimer.Tick(Time::DeltaTime);

	auto polling = myHandSlam.GetLeftHand().GetGameObject()->GetSingletonComponent<PollingStationComponent>();
	const auto playerPos = polling->GetPlayerPos();

	auto& modelTransform = myHandSlam.GetLeftHand().GetTransform();

	const Vec3f curvePos = myBezier.Evaluate(myTimer.Percent());
	modelTransform.SetPosition(curvePos);
	// modelTransform.LookAt(playerPos);
	*/
}

bool FB::HandSlam_End::IsFinished()
{
	return myIsFinished;
	//return myTimer.IsFinished();
}

void FB::HandSlam_End::OnEnter()
{
	myHandSlam.GetLeftHand().GetAnimator().GetController().SetInt(Constants::SlamStateIntName, 4);

	myIsFinished = false;

	// myTimer = TickTimer::FromSeconds(2.f * myHandSlam.GetLeftHand().GetEnragedDurationMultiplier());
	// 
	// myBezier = {};
	// myBezier.AddPoint(myHandSlam.GetLeftHand().GetTransform().GetPosition());
	// myBezier.AddPoint(myHandSlam.GetInitialPosition());
}

void FB::HandSlam_End::OnExit()
{
}
