#pragma once

#include "Component.h"

namespace Engine { class EnvironmentLightComponent; }
namespace Engine { class ModelComponent; }

namespace Engine 
{
	class FolliageCuller : public Component
	{
	public:
		COMPONENT(FolliageCuller, "FolliageCuller");

		FolliageCuller() = default;
		FolliageCuller(GameObject* aGameObject);

		void Start() override;

		void Render() override;

		void Reflect(Reflector& aReflector) override;

	private:
		EnvironmentLightComponent* myEnvLight = nullptr;
		ModelComponent* myModel = nullptr;

		float myDistance = 0.f;
		bool myDistanceOverridden = false;
	};
}