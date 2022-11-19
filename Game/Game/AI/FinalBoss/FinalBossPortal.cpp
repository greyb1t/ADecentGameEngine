#include "pch.h"
#include "FinalBossPortal.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/ModelComponent.h"
#include "AI/FinalBoss/FinalBoss.h"
#include "Engine/ResourceManagement/Resources/AnimationCurveResource.h"
#include "Engine/AnimationCurve/Curve.h"

void FB::FinalBossPortal::Start()
{
	if (!myPlaneRef.IsValid())
	{
		LOG_ERROR(LogType::Game) << "Missing plane ref in Final Boss Portal";
		myGameObject->Destroy();
		return;
	}

	if (!myFinalBossRef.IsValid())
	{
		LOG_ERROR(LogType::Game) << "Missing final boss ref in Final Boss Portal";
		myGameObject->Destroy();
		return;
	}

	myFinalBoss = myFinalBossRef.Get()->GetComponent<FinalBoss>();

	if (!myFinalBoss)
	{
		LOG_ERROR(LogType::Game) << "Missing Final Boss component in Final Boss Portal";
		myGameObject->Destroy();
		return;
	}

	myPortalPlaneModel = myPlaneRef.Get()->GetComponent<Engine::ModelComponent>();

	if (!myPortalPlaneModel)
	{
		LOG_ERROR(LogType::Game) << "Missing ModelComponent component in portal plane";
		myGameObject->Destroy();
		return;
	}
}

void FB::FinalBossPortal::Execute(Engine::eEngineOrder aOrder)
{
	switch (myState)
	{
	case State::Transitioning:
	{
		myTransitionTimer.Tick(Time::DeltaTime);

		const float intensity = Math::Lerp(myStartIntensity, myTargetIntensity, myTransitionTimer.Percent());

		float t = 1.f;

		if (myIntensityCurve && myIntensityCurve->IsValid())
		{
			t = myIntensityCurve->Get().Evaluate(myTransitionTimer.Percent());
		}
		else
		{
			LOG_ERROR(LogType::Game) << "Missing portal intensity curve";
		}

		SetPortalIntensity(intensity * t);

		if (myTransitionTimer.JustFinished())
		{
			myState = State::Sustaining;
		}
	} break;
	case State::Sustaining:
	{
		mySustainTimer.Tick(Time::DeltaTime);

		if (mySustainTimer.JustFinished())
		{
			const float intensity = Random::RandomFloat(myIntensityMinMax.x, myIntensityMinMax.y);

			const float transitionDuration = Random::RandomFloat(myPulseTransitionDurationMinMax.x, myPulseTransitionDurationMinMax.y);

			const float sustainDuration = Random::RandomFloat(
				mySustainIntensityDurationMinMax.x,
				mySustainIntensityDurationMinMax.y);

			Pulse(intensity, transitionDuration, sustainDuration);
		}
	} break;
	case State::None:
	{
		/*
		const bool isBossStarted = myFinalBoss->ShouldStartBoss();

		if (isBossStarted)
		{
			// Start it
			Pulse(10.f, 1.f, 1.f);
		}
		*/
	} break;
	default:
		break;
	}
}

void FB::FinalBossPortal::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myPlaneRef, "Plane");
	aReflector.Reflect(myFinalBossRef, "Final Boss");
	aReflector.Reflect(myIntensityCurve, "Pulse Curve");

	aReflector.Reflect(myPulseTransitionDurationMinMax, "Pulse Transition Duration MinMax");
	aReflector.Reflect(mySustainIntensityDurationMinMax, "Sustain Intensity Duration MinMax");
	aReflector.Reflect(myIntensityMinMax, "Intensity MinMax");
}

void FB::FinalBossPortal::Pulse(
	const float aIntensity,
	const float aTransitionDurationSec,
	const float aSustainDurationSec)
{
	myTargetIntensity = aIntensity;

	mySustainTimer = TickTimer::FromSeconds(aSustainDurationSec);
	myTransitionTimer = TickTimer::FromSeconds(aTransitionDurationSec);

	if (myPortalPlaneModel)
	{
		myStartIntensity = myPortalPlaneModel->GetMeshMaterialInstanceByIndex(0).GetAdditiveIntensity();
	}
	else
	{
		myStartIntensity = 1.f;
	}

	myState = State::Transitioning;
}

void FB::FinalBossPortal::SetPortalColor(const Vec3f& aColor)
{
	if (myPortalPlaneModel)
	{
		myPortalPlaneModel->GetMeshMaterialInstanceByIndex(0).SetAdditiveColor(aColor);
	}
}

void FB::FinalBossPortal::SetPortalIntensity(const float aIntensity)
{
	if (myPortalPlaneModel)
	{
		myPortalPlaneModel->GetMeshMaterialInstanceByIndex(0).SetAdditiveIntensity(aIntensity);
	}
}
