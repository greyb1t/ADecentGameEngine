#include "pch.h"
#include "HandGattlingEye.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "AI/FinalBoss/RightProjectileHand.h"
#include "AI/FinalBoss/Constants.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"
#include "Engine/GameObject/GameObject.h"
#include "AI/FinalBoss/States/HandGatlingEye/HandGatlingEye_Start.h"
#include "AI/FinalBoss/States/HandGatlingEye/HandGatlingEye_Loop.h"
#include "AI/FinalBoss/States/HandGatlingEye/HandGatlingEye_End.h"

FB::HandGattlingEye::HandGattlingEye(RightProjectileHand& aRightHand)
	: myRightHand(aRightHand)
{
	myStates[HandGattlingEyeStates::Start] = MakeOwned<HandGatlingEye_Start>(*this);
	myStates[HandGattlingEyeStates::Loop] = MakeOwned<HandGatlingEye_Loop>(*this);
	myStates[HandGattlingEyeStates::End] = MakeOwned<HandGatlingEye_End>(*this);

	myStateSequencer.AddState(myStates[HandGattlingEyeStates::Start].get());
	myStateSequencer.AddState(myStates[HandGattlingEyeStates::Loop].get());
	myStateSequencer.AddState(myStates[HandGattlingEyeStates::End].get());
}

void FB::HandGattlingEye::Update()
{
	myStateSequencer.Update();
}

bool FB::HandGattlingEye::IsFinished() const
{
	return myStateSequencer.IsFinished();
}

void FB::HandGattlingEye::OnEnter()
{
	myStateSequencer.Init();
}

void FB::HandGattlingEye::OnExit()
{
	myStateSequencer.OnExit();
}

void FB::HandGattlingEye::Reflect(Engine::Reflector& aReflector)
{
	HandGattlingEyeStates myStateType = HandGattlingEyeStates::_from_integral(myStateSequencer.GetActiveStateIndex());
	aReflector.ReflectEnum(myStateType, "State");

	if (myStateSequencer.GetActiveState())
	{
		myStateSequencer.GetActiveState()->Reflect(aReflector);
	}
}

FB::RightProjectileHand& FB::HandGattlingEye::GetRightHand()
{
	return myRightHand;
}

bool FB::HandGattlingEye::IsAllowedToMoveExternally() const
{
	return false;
}
