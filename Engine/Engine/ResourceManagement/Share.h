#pragma once

#include "ResourceEnums.h"

namespace Engine
{
	class ResourceBase;

	struct Share
	{
		std::mutex myGlobalResourceMutex;
		//std::vector<Owned<std::unordered_map<std::string, Shared<ResourceBase>>>> myResources;
		std::array<std::unordered_map<std::string, Shared<ResourceBase>>, static_cast<int>(ResourceType::Count)> myResources;

		std::mutex myGlobalResourcesToLoadMutex;
		//std::vector<Owned<std::vector<Owned<ResourceBase>>>> myResourcesToLoad;
		std::array<std::vector<Owned<ResourceBase>>, static_cast<int>(ResourceType::Count)> myResourcesToLoad;
	};
}