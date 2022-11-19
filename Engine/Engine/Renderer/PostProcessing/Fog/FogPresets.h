#pragma once

#include "Fog.h"

namespace Engine
{
	class FogPresets
	{
	public:
		void Deserialize(nlohmann::json& aJson);

		const Fog& GetFog(const std::string& aName) const;

	private:
		std::unordered_map<std::string, Fog> myFogPresets;
	};
}