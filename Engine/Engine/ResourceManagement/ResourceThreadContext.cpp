#include "pch.h"
#include "ResourceThreadContext.h"

#include "Engine/Renderer/Texture/TextureLoader.h"
#include "ResourceManager.h"
#include "ResourceBase.h"

namespace Engine
{
	ResourceThreadContext::ResourceThreadContext(const ResourceManagerSettings& aResourceManagerSettings)
		: myResourceManagerSettings(aResourceManagerSettings)
	{
	}

	ResourceThreadContext::~ResourceThreadContext()
	{
	}

	bool ResourceThreadContext::Init(
		ID3D11Device* aDevice,
		ID3D11DeviceContext& aContext,
		ResourceManager& aResourceManagerHandler)
	{
		if (!myResourceThreadData.Init(aDevice, aContext, aResourceManagerHandler))
		{
			LOG_ERROR(LogType::Resource) << "Failed to init render resource thread data";
			return false;
		}

		return true;
	}

	void ResourceThreadContext::TransferResourceToRenderThread(LdrThread& LoaderThread, Share& aShare)
	{
		{
			// Hopefully no deadlock here
			std::unique_lock<std::mutex> lock(LoaderThread.myResourcesToLoadMutex);
			std::unique_lock<std::mutex> lock2(aShare.myGlobalResourcesToLoadMutex);

			// If a call to GetResourceAsync() occured before a GetResourceNow()
			// we load the resource NOW and remove the resource that was requested earlier
			// therefore we need to remove them from the queue's here
			// myResourcesToLoad
			for (const auto& resourcePair : myResourceToRemoveFromLoadQueue)
			{
				const int resourceToRemoveTypeId = resourcePair.first;
				const ResourceId& resourceToRemove = resourcePair.second;

				// Remove on the Render thread
				{
					auto& resourceList = aShare.myResourcesToLoad[resourceToRemoveTypeId];

					auto pastEndIt = std::remove_if(
						resourceList.begin(),
						resourceList.end(),
						[&](Owned<ResourceBase>& aResource)
					{
						if (aResource->GetId() == resourceToRemove)
						{
							return true;
						}
						else
						{
							return false;
						}
					});

					resourceList.erase(pastEndIt, resourceList.end());
				}


				// Remove on the Loader thread
				{
					auto& resourceList = LoaderThread.myResourcesToLoad[resourceToRemoveTypeId];

					auto pastEndIt = std::remove_if(
						resourceList.begin(),
						resourceList.end(),
						[&](Owned<ResourceBase>& aResource)
					{
						if (aResource->GetId() == resourceToRemove)
						{
							return true;
						}
						else
						{
							return false;
						}
					});

					resourceList.erase(pastEndIt, resourceList.end());
				}
			}
		}
	}
}
