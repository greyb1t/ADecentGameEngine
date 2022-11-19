#include "pch.h"
#include "ExtraSceneJsonData.h"

bool Engine::ExtraSceneJsonData::InitFromJson(const Path& aExtraJsonPath)
{
	std::ifstream file(aExtraJsonPath.ToWString());

	if (!file.is_open())
	{
		LOG_ERROR(LogType::Animation) << "Failed to open " << aExtraJsonPath;
		return false;
	}

	const bool ignoreComments = true;
	const bool allowExceptions = false;
	nlohmann::json j = nlohmann::json::parse(file, nullptr, allowExceptions, ignoreComments);

	// If failed to parse json file
	if (j.is_discarded())
	{
		LOG_ERROR(LogType::Animation) << "Failed to parse " << aExtraJsonPath;
		return false;
	}

	defaultCubemapPath = j["DefaultCubemapPath"];
	defaultFogPresetName = j["DefaultFogPresetName"];

	cubemapColor = Vec3f(
		j["CubemapColor"][0],
		j["CubemapColor"][1],
		j["CubemapColor"][2]);

	cubemapColorIntensity = j["CubemapColorIntensity"];
	cubemapIntensity = j["CubemapIntensity"];

	shadowStrength = j["ShadowStrength"];

	return true;
}
