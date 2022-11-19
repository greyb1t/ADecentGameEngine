#include "pch.h"
#include "FinalBossLightning.h"
#include "Engine/Shortcuts.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/PostProcessComponent.h"
#include "Engine/GameObject/Components/EnvironmentLightComponent.h"
#include "Engine/Reflection/Reflector.h"

FinalBossLightning::FinalBossLightning()
{
}

FinalBossLightning::FinalBossLightning(GameObject* aGameObject)
	: Component(aGameObject)
{
}

FinalBossLightning::~FinalBossLightning()
{
}

void FinalBossLightning::Start()
{
	myEnvironmentLight = myGameObject->GetScene()->GetEnvironmentLight()->GetComponent<Engine::EnvironmentLightComponent>();

	if (auto pp = myGameObject->GetSingletonComponent<Engine::PostProcessComponent>())
	{
		myFog = &pp->GetFogSettingsRef();
	}
	else
	{
		myGameObject->Destroy();
		LOG_ERROR(LogType::Game) << "missing post process component";
		return;
	}

	myInitialFogColor = myFog->myColor;

	// myInitialPointLightIntensity = myPointLight->GetIntensity();
}

void FinalBossLightning::Execute(Engine::eEngineOrder aOrder)
{
	switch (myState)
	{
	case FinalBossLightning::WaitCooldown:
	{
		myCooldownTimer += Time::DeltaTime;

		if (myCooldownTimer >= myCooldownDuration)
		{
			myCooldownTimer = 0.f;

			myCooldownDuration = Random::RandomFloat(
				myCooldownDurationMinMax.x,
				myCooldownDurationMinMax.y);

			myState = State::IntensityGoDown;

			break;
		}
	} break;

	case FinalBossLightning::IntensityGoDown:
	{
		myIntensityTimer += Time::DeltaTime;

		if (myIntensityTimer >= myIntensityDuration)
		{
			myIntensityTimer = 0.f;

			myState = State::IntensityGoUp;

			break;
		}

		const float timerPercent = Math::InverseLerp(
			0.f, myIntensityDuration, myIntensityTimer);

		const auto fogColor = Math::Lerp(
			myInitialFogColor, myTargetFogColor, timerPercent);

		myFog->myColor = fogColor;

	} break;

	case FinalBossLightning::IntensityGoUp:
	{
		myIntensityTimer += Time::DeltaTime;

		if (myIntensityTimer >= myIntensityDuration)
		{
			myIntensityTimer = 0.f;

			myIntensityDuration = Random::RandomFloat(
				myIntensityDurationMinMax.x,
				myIntensityDurationMinMax.y);

			myState = State::WaitCooldown;

			break;
		}

		const float timerPercent = Math::InverseLerp(
			0.f, myIntensityDuration, myIntensityTimer);

		const Vec3f fogColor = Math::Lerp(
			myTargetFogColor, myInitialFogColor, timerPercent);

		myFog->myColor = fogColor;
	} break;

	default:
		assert(false);
		break;
	}
}

void FinalBossLightning::Reflect(Engine::Reflector& aReflector)
{
	aReflector.SetNextItemTooltip("A random value between min and max will "
		"be used to determine how quickly light will lerp intensity");
	aReflector.SetNextItemRange(0.f, 1.f);
	aReflector.SetNextItemSpeed(0.001f);
	aReflector.Reflect(myIntensityDurationMinMax, "Intensity Duration Range");

	aReflector.SetNextItemTooltip("A random value between min and max will "
		"be used to determine how long to wait until next intensity lerp");
	aReflector.SetNextItemRange(0.f, 10.f);
	aReflector.SetNextItemSpeed(0.001f);
	aReflector.Reflect(myCooldownDurationMinMax, "Cooldown Duration Range");

	aReflector.Reflect(myTargetFogColor, "Target Fog Color");
}