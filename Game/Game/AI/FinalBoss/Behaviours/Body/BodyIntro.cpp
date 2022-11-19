#include "pch.h"
#include "BodyIntro.h"
#include "AI/FinalBoss/Constants.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "AI/FinalBoss/FinalBossBody.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/CameraShakeComponent.h"
#include "AI/FinalBoss/FinalBoss.h"

FB::BodyIntro::BodyIntro(FinalBossBody& aBody)
	: myBody(aBody)
{
	myBody.GetAnimator().GetController().AddStateOnExitCallback("Base", "Intro",
		[this]()
		{
			myIsFinished = true;
		});

	myBody.GetAnimator().GetController().AddEventCallback("CameraShakeScream",
		[this]()
		{
			auto mainCam = myBody.GetGameObject()->GetScene()->GetMainCameraGameObject();
			auto shakeComponent = mainCam->GetComponent<Engine::CameraShakeComponent>();
			if (shakeComponent)
			{
				shakeComponent->AddPerlinShake("FinalBossIntroScream");
			}
		});
}

void FB::BodyIntro::Update()
{
	myFailsafeTimer.Tick(Time::DeltaTime);

	// Just to ensure if it is ever stuck, make it unstuck
	if (myFailsafeTimer.JustFinished())
	{
		myIsFinished = true;
	}
}

bool FB::BodyIntro::IsFinished() const
{
	return myIsFinished;
}

void FB::BodyIntro::OnEnter()
{
	myFailsafeTimer = TickTimer::FromSeconds(15.f);

	myIsFinished = false;

	myBody.GetAnimator().GetController().Trigger(Constants::IntroTriggerName);
}

void FB::BodyIntro::OnExit()
{
	myIsFinished = true;
	myBody.SetIsIntroFinished(true);
}

void FB::BodyIntro::Reflect(Engine::Reflector& aReflector)
{
}
