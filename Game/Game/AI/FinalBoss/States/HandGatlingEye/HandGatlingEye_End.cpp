#include "pch.h"
#include "HandGatlingEye_End.h"
#include "AI/FinalBoss/Behaviours/HandGattlingEye.h"
#include "AI/FinalBoss/LeftPunchHand.h"
#include "Engine/GameObject/GameObject.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "AI/FinalBoss/Constants.h"
#include "AI/FinalBoss/RightProjectileHand.h"

FB::HandGatlingEye_End::HandGatlingEye_End(HandGattlingEye& aParent)
	: myEye(aParent)
{
	aParent.GetRightHand().GetAnimator().GetController().AddStateOnExitCallback("Base", "Gatling End",
		[this]()
		{
			myIsFinished = true;
		});
}

void FB::HandGatlingEye_End::Update()
{
}

bool FB::HandGatlingEye_End::IsFinished()
{
	return myIsFinished;
}

void FB::HandGatlingEye_End::OnEnter()
{
	myEye.GetRightHand().GetAnimator().GetController().SetInt(Constants::GatlingEyeStateIntName, 2);

	auto polling = myEye.GetRightHand().GetGameObject()->GetSingletonComponent<PollingStationComponent>();
	const auto playerPos = polling->GetPlayerPos();

	myIsFinished = false;
}

void FB::HandGatlingEye_End::OnExit()
{
}
