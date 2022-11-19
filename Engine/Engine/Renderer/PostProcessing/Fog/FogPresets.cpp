#include "pch.h"
#include "FogPresets.h"

void Engine::FogPresets::Deserialize(nlohmann::json& aJson)
{
	myFogPresets.clear();

	for (const auto& fogJson : aJson["Presets"])
	{
		Fog fog(fogJson["Name"]);

		FogData fogData;
		{
			fogData.myColor = Vec3f(
				fogJson["FogColor"][0],
				fogJson["FogColor"][1],
				fogJson["FogColor"][2]);

			fogData.myStart = fogJson["FogStart"];
			fogData.myEnd = fogJson["FogEnd"];

			fogData.myFogStrength = fogJson["FogStrength"];
		}

		fog.SetData(fogData);

		if (myFogPresets.find(fog.GetName()) != myFogPresets.end())
		{
			LOG_WARNING(LogType::Engine) << "Duplicate fog exists: " << fog.GetName();
		}

		myFogPresets[fog.GetName()] = fog;
	}
}

const Engine::Fog& Engine::FogPresets::GetFog(const std::string& aName) const
{
	assert(myFogPresets.find(aName) != myFogPresets.end() && "does not exist");

	return myFogPresets.at(aName);
}
