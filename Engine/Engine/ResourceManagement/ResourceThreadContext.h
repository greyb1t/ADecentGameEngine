#pragma once

#include "ResourceRef.h"
#include "ResourceManagerSettings.h"
#include "ResourceId.h"

#include "ResourceThreadData.h"

namespace Engine
{
	class ResourceManager;

	struct LdrThread;
	struct Share;

	class ResourceThreadContext
	{
	public:
		ResourceThreadContext(const ResourceManagerSettings& aResourceManagerSettings);
		~ResourceThreadContext();

		// Called from render thread
		bool Init(ID3D11Device* aDevice, ID3D11DeviceContext& aContext, ResourceManager& aResourceManagerHandler);

		// To avoid using a mutex or atomic variable, we at one point sync the render and loading thread
		// to move ownership of the resources
		// Called from render thread
		void TransferResourceToRenderThread(LdrThread& aLdrThread, Share& aShare);

	public:
		friend class ResourceBase;

		const ResourceManagerSettings myResourceManagerSettings;

		// int is ResourceTypeId
		std::vector<std::pair<int, ResourceId>> myResourceToRemoveFromLoadQueue;

		// Since we have a PostCreation() method
		// it may need the thread data as well
		// therefore we create on the the main thread as well
		ResourceThreadData myResourceThreadData;
	};
}