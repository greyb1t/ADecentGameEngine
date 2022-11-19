#include "pch.h"
#include "CameraShakeManager.h"
#include "PerlinShake.h"
#include "KickShake.h"

bool Engine::CameraShakeManager::Init(JsonManager& aJsonManager)
{
	const std::string path = "Assets\\Json\\CameraShakes.json";

	myShakesJson = aJsonManager.DeserializeAs<ShakesJson>(path);

	return true;
}

const Engine::PerlinShakeDesc& Engine::CameraShakeManager::GetPerlinShake(const std::string& aName) const
{
	auto f = myShakesJson->myPerlinShakes.find(aName);

	if (f != myShakesJson->myPerlinShakes.end())
	{
		return *f->second;
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Missing perlin shake: " << aName;
	}

	static PerlinShakeDesc default;
	return default;
}

const Engine::KickShakeDesc& Engine::CameraShakeManager::GetKickShake(const std::string& aName) const
{
	auto f = myShakesJson->myKickShakes.find(aName);

	if (f != myShakesJson->myKickShakes.end())
	{
		return *f->second;
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Missing kick shake: " << aName;
	}

	static KickShakeDesc default;
	return default;
}

const std::unordered_map<std::string, Engine::PerlinShakeDesc*>& Engine::CameraShakeManager::GetPerlinShakes() const
{
	return myShakesJson->myPerlinShakes;
}

std::vector<std::string> Engine::CameraShakeManager::GetPerlinShakeNames() const
{
	std::vector<std::string> names;
	for (auto& p : myShakesJson->myPerlinShakes)
	{
		names.push_back(p.first);
	}
	return names;
}

std::vector<std::string> Engine::CameraShakeManager::GetKickShakeNames() const
{
	std::vector<std::string> names;
	for (auto& p : myShakesJson->myKickShakes)
	{
		names.push_back(p.first);
	}
	return names;
}

void Engine::ShakesJson::Deserialize(nlohmann::json& aJson)
{
	for (const auto& perlinShakeJson : aJson["PerlinShakes"])
	{
		const std::string name = perlinShakeJson["Name"];

		if (myPerlinShakes.find(name) != myPerlinShakes.end())
		{
			// If found, update existing to avoid re-allocing the
			// pointer and ruining auto-update from JSON if someone
			// has saved the pointer from GetPerlinShake()
			myPerlinShakes[name]->InitFromJson(perlinShakeJson);
		}
		else
		{
			// If not found, allocate new
			PerlinShakeDesc desc;
			desc.InitFromJson(perlinShakeJson);

			myPerlinShakes[name] = DBG_NEW PerlinShakeDesc(desc);
		}
	}

	if (aJson.contains("KickShakes"))
	{
		for (const auto& kickShakeJson : aJson["KickShakes"])
		{
			const std::string name = kickShakeJson["Name"];

			if (myKickShakes.find(name) != myKickShakes.end())
			{
				// If found, update existing to avoid re-allocing the
				// pointer and ruining auto-update from JSON if someone
				// has saved the pointer from GetPerlinShake()
				myKickShakes[name]->InitFromJson(kickShakeJson);
			}
			else
			{
				// If not found, allocate new
				KickShakeDesc desc;
				desc.InitFromJson(kickShakeJson);

				myKickShakes[name] = DBG_NEW KickShakeDesc(desc);
			}
		}
	}
}
