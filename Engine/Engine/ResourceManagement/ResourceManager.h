#pragma once

#include "ResourceManagerSettings.h"
#include "ResourceRef.h"
#include "ResourceId.h"
#include "ResourceThreadContext.h"
#include "Share.h"
#include "ResourceEnums.h"

class FileWatcher;

namespace Engine
{
	class GraphicsEngine;
	class Texture2D;
}

namespace Engine
{
	struct LdrThread
	{
		std::mutex myLoadedResourcesMutex;
		//std::vector<Owned<std::unordered_map<std::string, Owned<ResourceBase>>>> myLoadedResources;
		std::array<std::unordered_map<std::string, Owned<ResourceBase>>, static_cast<int>(ResourceType::Count)> myLoadedResources;

		std::condition_variable myResourcesToLoaderCv;
		std::mutex myResourcesToLoadMutex;
		//std::vector<Owned<std::vector<Owned<ResourceBase>>>> myResourcesToLoad;
		std::array<std::vector<Owned<ResourceBase>>, static_cast<int>(ResourceType::Count)> myResourcesToLoad;
	};

	class ResourceManager
	{
	public:
		ResourceManager(const ResourceManagerSettings& aResourceManagerSettings);

		~ResourceManager();

		bool Init(GraphicsEngine& aGraphicsEngine, FileWatcher* aFileWatcher);

		void SetIsResourceLoadingAllowed(const bool aIsAllowed);

		template <typename T, typename... Args>
		ResourceRef<T> CreateRef(std::string aResourcePath, Args&&... aArgs);
		template <typename T, typename... Args>
		ResourceRef<T> CreateRefWithID(std::string aResourcePath, const ResourceId& aId, Args&&... aArgs);

		template <typename T>
		void RequestLoading(const ResourceId& aResourceId);

		template <typename T>
		void Load(const ResourceId& aResourceId);

		void TransferResourceToRenderThread();

		void LoadingThread(ResourceThreadData* aThreadData);

		template <typename T>
		static int GetResourceType();

		FileWatcher* GetFileWatcher();
		GraphicsEngine& GetGraphicsEngine();

		Shared<Texture2D> GetPlaceholderTexture();

		const ResourceManagerSettings& GetSettings() const;

		bool IsAllResourcesLoaded() const;

		bool CreateThreadContext(const std::thread::id aThreadId);

	private:
		std::vector<std::thread> myThreads;

		// inline static int myResourceTypeCounter = 0;

		std::unordered_map<std::thread::id, ResourceThreadContext*> myContexts;

		std::atomic_bool myRunning = true;

		std::atomic_bool myIsAllResourcesLoaded = true;

		const ResourceManagerSettings myResourceManagerSettings;

		FileWatcher* myFileWatcher = nullptr;

		// The texture to display while waiting the real texture to load
		Shared<Texture2D> myPlaceholderTexture = nullptr;

		LdrThread LoaderThread;

		Share myShare;

		// CAREFUL: Not allowed to use on anything other than main thread
		GraphicsEngine* myGraphicsEngine = nullptr;

		std::atomic_bool myIsResourceLoadingAllowed = true;

		std::array<std::atomic_int, static_cast<int>(ResourceType::Count)> myResourceIDCounters;
	};

	template <typename T, typename... Args>
	ResourceRef<T> ResourceManager::CreateRef(std::string aResourcePath, Args&&... aArgs)
	{
		std::replace(aResourcePath.begin(), aResourcePath.end(), '/', '\\');

		return CreateRefWithID<T, Args...>(aResourcePath, aResourcePath, std::forward<Args>(aArgs)...);
	}

	template <typename T, typename... Args>
	ResourceRef<T> ResourceManager::CreateRefWithID(std::string aResourcePath,
		const ResourceId& aId, Args&&... aArgs)
	{
		// We need all paths to be same slashes, otherwise the could be duplicates
		std::replace(aResourcePath.begin(), aResourcePath.end(), '/', '\\');

		// NOTE: Remove the identifer requirement because its no longer needed
		// to avoid adjusting too much code I simply do not remove it
		// at all, just from the arguments
		// const ResourceId aIdentifier = aResourcePath;

		std::unique_lock<std::mutex> lock(myShare.myGlobalResourceMutex);

		const auto resourceTypeId = GetResourceType<T>();

		assert(resourceTypeId < myShare.myResources.size()
			&& "not enough resource type space, preload more");

		const auto findResult = myShare.myResources[resourceTypeId].find(aId);

		// Returns the existing resource, otherwise create a new one
		if (findResult != myShare.myResources[resourceTypeId].end())
		{
			return ResourceRef<T>(std::static_pointer_cast<T>(findResult->second));
		}
		else
		{
			// Create a new resource
			Shared<ResourceBase> resource
				= MakeShared<T>(*this, aResourcePath, aId, std::forward<Args>(aArgs)...);

			// resource->myResourceId = resourceTypeId;
			resource->myType = static_cast<ResourceType>(resourceTypeId);

			resource->myID = myResourceIDCounters[resourceTypeId]++;

			myShare.myResources[resourceTypeId][aId] = resource;

			// Return the resource owned by the render thread, that later on gets updated on the sync-point
			// We must NOT return any internal loader thread data to avoid race conditions
			// That'd be crazy
			return ResourceRef<T>(std::static_pointer_cast<T>(resource));
		}
	}

	template <typename T>
	void ResourceManager::RequestLoading(const ResourceId& aResourceId)
	{
		if (!myIsResourceLoadingAllowed)
		{
			assert(false && "not allowed to load resource");
			return;
		}

		const auto resourceTypeId = GetResourceType<T>();

		//	// SANITY CHECK
		//	{
		//		std::unique_lock<std::mutex> lock(myShare.myGlobalResourcesToLoadMutex);
		//
		//		assert(resourceTypeId < myShare.myResourcesToLoad.size()
		//			&& "not enough resource type space, preload more");
		//
		//#ifdef _DEBUG
		//		for (const auto& resBase : *myShare.myResourcesToLoad[resourceTypeId])
		//		{
		//			if (resBase->GetId() == aResourceId)
		//			{
		//				assert(false && "must not exist");
		//			}
		//		}
		//#endif
		//	}

			// Copy the resources that was previously constructed
			// this removed the constructor arguments again :)
		const ResourceBase* res1;

		{
			std::unique_lock<std::mutex> lock(myShare.myGlobalResourceMutex);
			res1 = myShare.myResources[resourceTypeId][aResourceId].get();
		}

		const T* res2 = static_cast<const T*>(res1);

		auto resource = MakeOwned<T>(*res2);
		//resource->myResourceId = resourceTypeId;
		resource->myType = static_cast<ResourceType>(resourceTypeId);

		{
			std::unique_lock<std::mutex> lock(myShare.myGlobalResourcesToLoadMutex);
			myShare.myResourcesToLoad[resourceTypeId].push_back(std::move(resource));
		}
	}

	template <typename T>
	void ResourceManager::Load(const ResourceId& aResourceId)
	{
		std::vector<ResourceRef<ResourceBase>> children;

		{
			if (!myIsResourceLoadingAllowed)
			{
				assert(false && "not allowed to load resource");
				return;
			}

			const auto resourceTypeId = GetResourceType<T>();
			auto& resourceThreadData = myContexts.at(std::this_thread::get_id());

			std::unique_lock<std::mutex> lock(myShare.myGlobalResourcesToLoadMutex);

			assert(resourceTypeId < myShare.myResourcesToLoad.size()
				&& "not enough resource type space, preload more");

			ResourceBase* res1 = myShare.myResources[resourceTypeId][aResourceId].get();

			if (res1->myState == ResourceState::Requested)
			{
				// If we came here, it has not yet been loaded by the Async call
				// We must load the resource NOW and prevent the resource
				// to be loaded on the loader threads that was added by the Async call
				const auto toRemove = std::make_pair(resourceTypeId, aResourceId);
				resourceThreadData->myResourceToRemoveFromLoadQueue.push_back(toRemove);
			}

			T* res2 = static_cast<T*>(res1);

			// Copy the resources that was previously constructed
			// this removed the constructor arguments again :)
			// auto resource = MakeOwned<T>(*res2);
			// resource->myResourceId = resourceTypeId;

			lock.unlock();

			// Do not allocate a new one, we need to use the one already there
			// otherwise the previous call to GetResourceAsync would
			// have a pointer to a non-existing resource
			// TODO: don't need MakeShared here, because we are only adjusting the resourceFound variable
			// Shared<ResourceBase> tempResource = MakeShared<T>(*this, aResourcePath, aIdentifier, std::forward<Args>(aArgs)...);

			// Copy the resources that was previously constructed
			// this removed the constructor arguments again :)
			// TODO: REMOVE MAKEOWNED
			auto tempResource = MakeOwned<T>(*res2);
			tempResource->myType = static_cast<ResourceType>(resourceTypeId);
			//tempResource->myResourceId = resourceTypeId; // why?

			const bool valid = tempResource->Create(resourceThreadData->myResourceThreadData);

			if (valid)
			{
				// non recursive, because it will become recursive automatically because we call Load()
				// on the children at end of function
				children = tempResource->GetChildren(false);

				LOG_INFO(LogType::Resource)
					<< "Loaded (" << std::this_thread::get_id() << ") " << tempResource->GetPath();
			}
			else
			{
				LOG_ERROR(LogType::Resource)
					<< "Failed load (" << std::this_thread::get_id() << ") " << tempResource->GetPath();
			}

			lock.lock();

			res1->myLoadResult
				= (valid == true) ? ResourceLoadResult::Succeeded : ResourceLoadResult::Failed;
			// res1->myResourceId = resourceTypeId;
			res1->myType = static_cast<ResourceType>(resourceTypeId);
			res1->Swap(tempResource.get());
			res1->PostCreation(resourceThreadData->myResourceThreadData);
		}

		// Load the children
		for (int i = 0; i < children.size(); ++i)
		{
			children[i]->Load();
		}
	}

	template <typename T>
	int ResourceManager::GetResourceType()
	{
		return static_cast<int>(T::ourResourceType);
		//static int id = myResourceTypeCounter++;
		//return id;
	}
}