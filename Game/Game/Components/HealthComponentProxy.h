#pragma once

#include "HealthComponent.h"

namespace Engine
{
	class GameObjectRef;
}

namespace FB
{
	class HealthComponentProxy : public HealthComponent
	{
	public:
		COMPONENT(HealthComponentProxy, "Health Component Proxy");

		HealthComponentProxy() = default;
		HealthComponentProxy(GameObject* aGameObject);

		void Start() override;

		void Execute(Engine::eEngineOrder aOrder) override;

		void ApplyDamage(
			float aAmount,
			bool aIgnoreShield = false,
			const CU::Vector3f* anImpactPosition = nullptr, 
			eDamageType aType = eDamageType::Basic) override;

		void Reflect(Engine::Reflector& aReflector) override;

	private:
		HealthComponent* myParentHealthComponent = nullptr;
		Engine::GameObjectRef myParentGameObject;
	};
}