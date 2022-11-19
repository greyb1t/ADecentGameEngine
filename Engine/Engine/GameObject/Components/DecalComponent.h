#pragma once

#include "Component.h"
#include "Engine\Renderer\Material\DecalMaterialInstance.h"

class GameObject;

namespace Engine
{
	class DecalComponent : public Component
	{
	public:
		COMPONENT(DecalComponent, "Decal");

		DecalComponent() = default;
		DecalComponent(GameObject* aGameObject);

		void Start() override;

		void Execute(eEngineOrder aOrder) override;

		void Render() override;

		void Reflect(Reflector& aReflector) override;

		DecalMaterialInstance& GetMaterialInstance();

	private:
		// The box size
		Vec3f mySize = Vec3f(1.f, 1.f, 1.f);

		DecalMaterialInstance myMaterialInstance;
	};
}