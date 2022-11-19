#pragma once

#include "Components/HUD/HealthBarComponent.h"

namespace FB
{
	class FinalBossHealthbar : public HealthBarComponent
	{
		COMPONENT(FinalBossHealthbar, "FB HealthBarComponent");
	public:
		void Start() override;

		void Reflect(Engine::Reflector& aReflector) override;

	private:
		Engine::GameObjectRef myHealthTargetGameObject;
	};
}