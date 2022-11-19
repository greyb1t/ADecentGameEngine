#pragma once

#include "Component.h"
#include "Engine/GameObject/GameObjectRef.h"

namespace Engine
{
	class SkyboxComponent : public Component
	{
		COMPONENT_SINGLETON(SkyboxComponent, "Skybox");
	public:
		SkyboxComponent() = default;
		SkyboxComponent(GameObject* aGameObject);


		void Awake() override;
		void Render() override;

		void SetAdditiveColor(const Vec3f& aColor);
		const Vec3f GetAdditiveColor() const;

	private:
		ModelComponent* myModelComponent = nullptr;
	};
}