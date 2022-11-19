#include "pch.h"
#include "AnimationTransformResult.h"

void Engine::AnimationTransformResult::InitFromJson(const nlohmann::json& aJson)
{
	for (const auto& value : aJson)
	{
		AnimationTransform transform;
		transform.InitFromJson(value);

		myEntries.push_back(transform);
	}
}

nlohmann::json Engine::AnimationTransformResult::ToJson() const
{
	nlohmann::json j = nlohmann::json::array();

	for (const auto& entry : myEntries)
	{
		j.push_back(entry.ToJson());
	}

	return j;
}
