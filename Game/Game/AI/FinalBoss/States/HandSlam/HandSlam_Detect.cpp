#include "pch.h"
#include "HandSlam_Detect.h"
#include "AI/FinalBoss/Behaviours/HandSlam.h"
#include "AI/FinalBoss/LeftPunchHand.h"
#include "Engine/GameObject/GameObject.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "AI/FinalBoss/Constants.h"

FB::HandSlam_Detect::HandSlam_Detect(HandSlam& aParent)
	: myHandSlam(aParent)
{
	myHandSlam.GetLeftHand().GetAnimator().GetController().AddStateOnExitCallback("Base", "Slam Detect",
		[this]()
		{
			myIsFinished = true;
		});

	myHandSlam.GetLeftHand().GetAnimator().GetController().AddEventCallback("StartTheFall",
		[this]()
		{
			myIsFinished = true;
		});
}

void FB::HandSlam_Detect::Update()
{
	//auto polling = myHandSlam.GetLeftHand().GetGameObject()->GetSingletonComponent<PollingStationComponent>();
	//const auto playerPos = polling->GetPlayerPos();
	//
	//auto& modelTransform = myHandSlam.GetLeftHand().GetTransform();
	//myHandSlam.GetLeftHand().SetTargetRotation(modelTransform.GetLookAtResult(playerPos));
}

bool FB::HandSlam_Detect::IsFinished()
{
	return myIsFinished;
}

void FB::HandSlam_Detect::OnEnter()
{
	myHandSlam.GetLeftHand().GetAnimator().GetController().SetInt(Constants::SlamStateIntName, 2);

	auto polling = myHandSlam.GetLeftHand().GetGameObject()->GetSingletonComponent<PollingStationComponent>();
	const auto playerPos = polling->GetPlayerPos();

	myIsFinished = false;
}

void FB::HandSlam_Detect::OnExit()
{
}
