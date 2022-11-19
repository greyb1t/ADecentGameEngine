#pragma once

#include "Engine/GameObject/Components/Component.h"

namespace Engine
{
	class PointLightComponent;
	class SpotLightComponent;
}

namespace FB
{
	class FinalBossShootFlash : public Component
	{
	public:
		COMPONENT(FinalBossShootFlash, "FB ShootFlash");

		enum State
		{
			IntensityGoUp,
			IntensityGoDown,
		};

		FinalBossShootFlash();
		FinalBossShootFlash(GameObject* aGameObject);
		~FinalBossShootFlash();

		void Start() override;

		void Execute(Engine::eEngineOrder aOrder) override;

		void Reflect(Engine::Reflector& aReflector) override;

	private:
		void DestroyMe();

	private:
		GameObject* myPointLightGo = nullptr;
		Engine::PointLightComponent* myPointLight = nullptr;

		float myInitialPointLightIntensity = 0.f;

		float myIntensityTimer = 0.f;
		float myIntensityDuration = 0.03f;

		State myState = State::IntensityGoUp;
	};
}