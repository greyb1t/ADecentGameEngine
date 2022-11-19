#pragma once

#include "Engine/GameObject/Components/Component.h"
#include "Engine/UIEventManager/UIEventManager.h"
#include <Physics/Shape.h>

namespace Engine
{
	class TriggerComponent : public Component
	{
	public:
		COMPONENT_COPYABLE(TriggerComponent, "TriggerComponent");

		TriggerComponent() = default;
		TriggerComponent(GameObject* aGameObject);
		TriggerComponent(GameObject* aGameObject, Vec3f aSize, std::string aLayer, std::string aEvent);

		void Start() override;

		void Execute(eEngineOrder aOrder) override;
		void Render() override;

		void Reflect(Reflector& aReflector) override;

		void SetLayer(std::string aLayer);
		void SetSize(Vec3f aSize);
		void SetEvent(std::string aEvent);

		const std::string& GetLayer();
		Vec3f GetSize();
		const std::string& GetEvent();

	private:
		unsigned int StringToLayer(std::string aLayer);

		Shape myShape;
		physx::PxTransform	myPxTransform;

		Vec3f mySize = { 100,100,100 };
		std::string myLayer;
		std::string myEvent;

		bool myIsInteractor = false;

		Vec3f myOffset{ 0, 0, 0 };

		float triggerCoooldown = 2.f;
		float triggerCoooldownTimer = 0.f;

	};
}