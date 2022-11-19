#pragma once

#include "Component.h"

namespace Engine
{
	class LandscapeComponent : public Component
	{
	public:
		COMPONENT(LandscapeComponent, "LandscapeComponent");

		LandscapeComponent() = default;
		LandscapeComponent(GameObject * aGameObject);
		LandscapeComponent(const LandscapeComponent& aOther) = default;

		void Reflect(Reflector& aReflector) override;

		void SetLandscape(const std::string& aPath);
		
	private:
		void LoadLandscape(const std::string& aPath);

		std::string myPath = "";
	};
}