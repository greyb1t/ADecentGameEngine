#include "pch.h"
#include "HandGatlingEye_Start.h"
#include "AI/FinalBoss/Behaviours/HandGattlingEye.h"
#include "AI/FinalBoss/LeftPunchHand.h"
#include "Engine/GameObject/GameObject.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "AI/FinalBoss/Constants.h"
#include "AI/FinalBoss/RightProjectileHand.h"
#include "AI/FinalBoss/FinalBoss.h"
#include "Engine/GameObject/Components/AudioComponent.h"

FB::HandGatlingEye_Start::HandGatlingEye_Start(HandGattlingEye& aParent)
	: myEye(aParent)
{
	myEye.GetRightHand().GetAnimator().GetController().AddStateOnExitCallback(
		"Base", "Gatling Start", [this]() { myIsFinished = true; });
}

void FB::HandGatlingEye_Start::Update()
{
	const auto playerPos = myEye.GetRightHand().GetFinalBoss().GetPlayerPos();

	auto r = myEye.GetRightHand().GetTransform().GetLookAtResult(playerPos);
	myEye.GetRightHand().SetTargetRotation(r);
}

bool FB::HandGatlingEye_Start::IsFinished()
{
	return myIsFinished;
}

void FB::HandGatlingEye_Start::OnEnter()
{
	myEye.GetRightHand().GetAnimator().GetController().Trigger(Constants::GatlingEyeTriggerName);
	myEye.GetRightHand().GetAnimator().GetController().SetInt(Constants::GatlingEyeStateIntName, 0);

	auto polling = myEye.GetRightHand().GetGameObject()->GetSingletonComponent<PollingStationComponent>();
	const auto playerPos = polling->GetPlayerPos();

	myIsFinished = false;

	GatlingLaserStartSound(true);
}

void FB::HandGatlingEye_Start::OnExit()
{
	GatlingLaserStartSound(false);
}

void FB::HandGatlingEye_Start::GatlingLaserStartSound(const bool aEnable) const
{
	if (aEnable)
	{
		myEye.GetRightHand().GetAudio().PlayEvent("GatlingLaserStart");
	}
	else
	{
		myEye.GetRightHand().GetAudio().StopEvent("GatlingLaserStart");
	}
}
