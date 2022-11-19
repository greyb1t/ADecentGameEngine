#include "pch.h"
#include "FogManager.h"
#include "FogPresets.h"

bool Engine::FogManager::Init(JsonManager& aJsonManager)
{
	myPresets = aJsonManager.DeserializeAs<FogPresets>("Assets\\Json\\FogPresets.json");

	if (myPresets == nullptr)
	{
		return false;
	}

	return true;
}

const Engine::FogPresets& Engine::FogManager::GetPresets()
{
	return *myPresets;
}

