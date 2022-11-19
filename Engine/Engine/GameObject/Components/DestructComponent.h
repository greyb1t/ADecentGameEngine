#pragma once

#include "Engine/GameObject/Components/Component.h"
#include "Engine/UIEventManager/UIEventManager.h"

namespace Engine
{
	class DestructComponent : public Component
	{
	public:
		COMPONENT_COPYABLE(DestructComponent, "DestructComponent");

		DestructComponent() = default;
		DestructComponent(GameObject* aGameObject, float aTime);

		virtual ~DestructComponent();

		void Start() override;
		void Execute(eEngineOrder aOrder) override;
		void Reflect(Reflector& aReflector) override;
	private:
		float myTimer = 0;
		float myTime = 0;
	};
}