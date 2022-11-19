#include "pch.h"
#include "FinalBossShootFlash.h"
#include "Engine/Shortcuts.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/PointLightComponent.h"
#include "Engine/GameObject/Components/SpotLightComponent.h"
#include "Engine/Reflection/Reflector.h"
#include "AI/PollingStation/PollingStationComponent.h"

FB::FinalBossShootFlash::FinalBossShootFlash()
{
}

FB::FinalBossShootFlash::FinalBossShootFlash(GameObject* aGameObject)
	: Component(aGameObject)
{
}

FB::FinalBossShootFlash::~FinalBossShootFlash()
{
}

void FB::FinalBossShootFlash::Start()
{
	auto pollingStation = myGameObject->GetSingletonComponent<PollingStationComponent>();

	myPointLightGo = pollingStation->GetPointlightFromPool();

	if (myPointLightGo == nullptr)
	{
		// if no more point lights where available in pool, just destroy this
		DestroyMe();
	}

	myPointLightGo->SetActive(true);
	myPointLight = myPointLightGo->GetComponent<Engine::PointLightComponent>();

	if (myPointLight)
	{
		myInitialPointLightIntensity = 1100157.625f;
	}
}

void FB::FinalBossShootFlash::Execute(Engine::eEngineOrder aOrder)
{
	myPointLightGo->GetTransform().SetPosition(GetTransform().GetPosition());

	myPointLight->SetColor(Vec3f(241.f / 255.f, 192.f / 255.f, 74.f / 255.f));
	myPointLight->SetRange(5000.f);

	switch (myState)
	{
	case FB::FinalBossShootFlash::IntensityGoUp:
	{
		myIntensityTimer += Time::DeltaTime;

		if (myIntensityTimer >= myIntensityDuration)
		{
			myIntensityTimer = 0.f;

			myState = State::IntensityGoDown;

			break;
		}

		const float timerPercent = Math::InverseLerp(
			0.f, myIntensityDuration, myIntensityTimer);

		if (myPointLight)
		{
			const float intensity = Math::Lerp(
				0.f, myInitialPointLightIntensity, timerPercent);

			myPointLight->SetIntensity(intensity);
		}
	} break;

	case FB::FinalBossShootFlash::IntensityGoDown:
	{
		myIntensityTimer += Time::DeltaTime;

		if (myIntensityTimer >= myIntensityDuration)
		{
			myIntensityTimer = 0.f;

			myPointLightGo->SetActive(false);

			DestroyMe();

			// myState = State::IntensityGoUp;

			break;
		}

		const float timerPercent = Math::InverseLerp(
			0.f, myIntensityDuration, myIntensityTimer);

		if (myPointLight)
		{
			const float intensity = Math::Lerp(
				myInitialPointLightIntensity, 0.f, timerPercent);

			myPointLight->SetIntensity(intensity);
		}
	} break;

	default:
		assert(false);
		break;
	}
}

void FB::FinalBossShootFlash::Reflect(Engine::Reflector& aReflector)
{
}

void FB::FinalBossShootFlash::DestroyMe()
{
	auto pollingStation = myGameObject->GetSingletonComponent<PollingStationComponent>();
	pollingStation->ReturnPointLightToPool(myPointLightGo);

	Destroy();
}
