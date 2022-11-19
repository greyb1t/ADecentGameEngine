#include "pch.h"
#include "HandSlam_Start.h"
#include "AI/FinalBoss/Behaviours/HandSlam.h"
#include "AI/FinalBoss/LeftPunchHand.h"
#include "Engine/GameObject/GameObject.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "AI/FinalBoss/Constants.h"

FB::HandSlam_Start::HandSlam_Start(HandSlam& aParent)
	: myHandSlam(aParent)
{
	myHandSlam.GetLeftHand().GetAnimator().GetController().AddStateOnExitCallback("Base", "Slam Start",
		[this]()
		{
			myIsFinished = true;
		});
}

void FB::HandSlam_Start::Update()
{
	// myTimer.Tick(Time::DeltaTime);

	auto polling = myHandSlam.GetLeftHand().GetGameObject()->GetSingletonComponent<PollingStationComponent>();
	const auto playerPos = polling->GetPlayerPos();

	auto& modelTransform = myHandSlam.GetLeftHand().GetTransform();
	myHandSlam.GetLeftHand().SetTargetRotation(modelTransform.GetLookAtResult(playerPos));
}

bool FB::HandSlam_Start::IsFinished()
{
	return myIsFinished;
}

void FB::HandSlam_Start::OnEnter()
{
	myHandSlam.GetLeftHand().GetAnimator().GetController().Trigger(Constants::SlamTriggerName);
	myHandSlam.GetLeftHand().GetAnimator().GetController().SetInt(Constants::SlamStateIntName, 0);

	myIsFinished = false;
}

void FB::HandSlam_Start::OnExit()
{
}
