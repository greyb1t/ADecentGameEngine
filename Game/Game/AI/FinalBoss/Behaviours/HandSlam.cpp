#include "pch.h"
#include "HandSlam.h"
#include "AI/FinalBoss/LeftPunchHand.h"
#include "AI/FinalBoss/States/HandSlam/HandSlam_Start.h"
#include "AI/FinalBoss/States/HandSlam/HandSlam_Detect.h"
#include "AI/FinalBoss/States/HandSlam/HandSlam_End.h"
#include "AI/FinalBoss/States/HandSlam/HandSlam_Fall.h"
#include "AI/FinalBoss/States/HandSlam/HandSlam_Search.h"
#include "Engine/GameObject/Components/AudioComponent.h"

FB::HandSlam::HandSlam(LeftPunchHand& aLeftHand)
	: myLeftHand(aLeftHand)
{
	myStates[HandSlamStateType::Start] = MakeOwned<HandSlam_Start>(*this);
	myStates[HandSlamStateType::Search] = MakeOwned<HandSlam_Search>(*this);
	myStates[HandSlamStateType::Detect] = MakeOwned<HandSlam_Detect>(*this);
	myStates[HandSlamStateType::Fall] = MakeOwned<HandSlam_Fall>(*this);
	myStates[HandSlamStateType::End] = MakeOwned<HandSlam_End>(*this);

	myStateSequencer.AddState(myStates[HandSlamStateType::Start].get());
	myStateSequencer.AddState(myStates[HandSlamStateType::Search].get());
	myStateSequencer.AddState(myStates[HandSlamStateType::Detect].get());
	myStateSequencer.AddState(myStates[HandSlamStateType::Fall].get());
	myStateSequencer.AddState(myStates[HandSlamStateType::End].get());
}

void FB::HandSlam::Update()
{
	myStateSequencer.Update();
}

bool FB::HandSlam::IsFinished() const
{
	return myStateSequencer.IsFinished();
}

FB::LeftPunchHand& FB::HandSlam::GetLeftHand()
{
	return myLeftHand;
}

const Vec3f& FB::HandSlam::GetInitialPosition() const
{
	return myInitialPosition;
}

const FB::HandSlamDesc& FB::HandSlam::Desc() const
{
	return myLeftHand.GetHandSlamDesc();
}

void FB::HandSlam::HandSlamSound(const bool aEnable) const
{
	if (aEnable)
	{
		myLeftHand.GetAudio().PlayEvent("HandSlam");
	}
	else
	{
		myLeftHand.GetAudio().StopEvent("HandSlam");
	}
}

void FB::HandSlam::HandSlamImpactSound(const bool aEnable) const
{
	if (aEnable)
	{
		myLeftHand.GetAudio().PlayEvent("HandSlamImpact");
	}
	else
	{
		myLeftHand.GetAudio().StopEvent("HandSlamImpact");
	}
}

void FB::HandSlam::OnEnter()
{
	myInitialPosition = myLeftHand.GetTransform().GetPosition();

	myStateSequencer.Init();
}

void FB::HandSlam::OnExit()
{
	myStateSequencer.OnExit();
}

void FB::HandSlam::Reflect(Engine::Reflector& aReflector)
{
	HandSlamStateType myStateType = HandSlamStateType::_from_integral(myStateSequencer.GetActiveStateIndex());
	aReflector.ReflectEnum(myStateType, "State");

	if (myStateSequencer.GetActiveState())
	{
		myStateSequencer.GetActiveState()->Reflect(aReflector);
	}
}