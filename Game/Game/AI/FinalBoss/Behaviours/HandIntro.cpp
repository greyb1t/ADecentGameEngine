#include "pch.h"
#include "HandIntro.h"
#include "AI/FinalBoss/Constants.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "AI/FinalBoss/FinalBossHand.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/VFXComponent.h"

FB::HandIntro::HandIntro(FinalBossHand& aHand)
	: myHand(aHand)
{
	myHand.GetAnimator().GetController().AddStateOnExitCallback("Base", "Intro",
		[this]()
		{
			myIsFinished = true;
		});
}

void FB::HandIntro::Update()
{
}

bool FB::HandIntro::IsFinished() const
{
	return myIsFinished;
}

void FB::HandIntro::OnEnter()
{
	myHand.GetAnimator().GetController().Trigger(Constants::IntroTriggerName);
	myIsFinished = false;
}

void FB::HandIntro::OnExit()
{
}

void FB::HandIntro::Reflect(Engine::Reflector& aReflector)
{
}

bool FB::HandIntro::IsAllowedToMoveExternally() const
{
	return false;
}
