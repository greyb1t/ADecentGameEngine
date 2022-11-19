#include "pch.h"
#include "BodyPhaseSwitch.h"
#include "AI/FinalBoss/Constants.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "AI/FinalBoss/FinalBossBody.h"

FB::BodyPhaseSwitch::BodyPhaseSwitch(FinalBossBody& aBody)
	: myBody(aBody)
{
	myBody.GetAnimator().GetController().AddStateOnExitCallback("Base", "PhaseSwitch",
		[this]()
		{
			myIsFinished = true;
		});
}

void FB::BodyPhaseSwitch::Update()
{
}

bool FB::BodyPhaseSwitch::IsFinished() const
{
	return myIsFinished;
}

void FB::BodyPhaseSwitch::OnEnter()
{
	myIsFinished = false;
	myBody.GetAnimator().GetController().Trigger(Constants::PhaseSwitchTriggerName);
}

void FB::BodyPhaseSwitch::OnExit()
{
}

void FB::BodyPhaseSwitch::Reflect(Engine::Reflector& aReflector)
{
}
