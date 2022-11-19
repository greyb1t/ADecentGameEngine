#pragma once

#include "Engine/Scene/FolderScene.h"

namespace Engine
{
class SceneRegistry
{
public:
	// Only called ONCE
	static void RegisterSceneTypes();

	static FolderScene* GetSceneTypeFromID(size_t aTypeId);

	static size_t GetIdFromTypeName(const std::string& aTypeName);

	static std::unordered_map<size_t, FolderScene*>& GetAllSceneTypes();

	static size_t GetSceneTypeCount();

	template <class T>
	static void RegisterType(const std::string& aName)
	{
		std::hash<std::string> hasher;

		const size_t hash = hasher(aName);

		myTypes[hash] = DBG_NEW T;
		myTypes[hash]->myTypeId = hash;
		myTypes[hash]->myTypeName = aName;
	}

	static void Destroy();

private:
	inline static std::unordered_map<size_t, FolderScene*> myTypes;
};

}
