#include "pch.h"
#include "SceneJson.h"

bool Engine::SceneJson::InitFromJson(const Path& aPath)
{
	std::ifstream file(aPath.ToWString());

	if (!file.is_open())
	{
		LOG_ERROR(LogType::Animation) << "Failed to open " << aPath;
		return false;
	}

	const bool ignoreComments = true;
	const bool allowExceptions = false;
	nlohmann::json j = nlohmann::json::parse(file, nullptr, allowExceptions, ignoreComments);

	// If failed to parse json file
	if (j.is_discarded())
	{
		LOG_ERROR(LogType::Animation) << "Failed to parse " << aPath;
		return false;
	}

	myName = j["Name"];

	return true;
}
