#include "pch.h"
#include "SceneRegistry.h"

#include "FolderScene.h"

void Engine::SceneRegistry::RegisterSceneTypes()
{
	// GET THE TYPE NAME AND THEN GET HASH OF THE TYPE NAME, THAT IS THE ID
	// TODO: Do this for NodeTypes/ComponentTypes as well, we dont want to keep the order

	// RegisterType<>("");
}

Engine::FolderScene* Engine::SceneRegistry::GetSceneTypeFromID(size_t aTypeId)
{
	if (myTypes.find(aTypeId) != myTypes.end())
	{
		return myTypes[aTypeId];
	}

	return nullptr;
}

size_t Engine::SceneRegistry::GetIdFromTypeName(const std::string& aTypeName)
{
	for (const auto& t : myTypes)
	{
		if (t.second->myTypeName == aTypeName)
		{
			return t.first;
		}
	}

	return -1;
}

std::unordered_map<size_t, Engine::FolderScene*>& Engine::SceneRegistry::GetAllSceneTypes()
{
	return myTypes;
}

size_t Engine::SceneRegistry::GetSceneTypeCount()
{
	return myTypes.size();
}

void Engine::SceneRegistry::Destroy()
{
	for (auto& [id, type] : myTypes)
	{
		delete type;
		type = nullptr;
	}
}
