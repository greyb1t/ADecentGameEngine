#include "pch.h"
#include "ResourceManager.h"
#include "Engine\Renderer\Texture\TextureLoader.h"
#include "ResourceBase.h"
#include "Engine/Renderer\GraphicsEngine.h"
#include "Engine/Renderer\Directx11Framework.h"
#include "Engine\Paths.h"
#include "Engine\Engine.h"

Engine::ResourceManager::ResourceManager(const ResourceManagerSettings& aResourceManagerSettings)
	: myResourceManagerSettings(aResourceManagerSettings)
{
}

Engine::ResourceManager::~ResourceManager()
{
	// Stop the loading thread
	myRunning = false;
	LoaderThread.myResourcesToLoaderCv.notify_all();

	// Manually destruct the thread pool FIRST before actually
	// destructing anything else in this class
	// Why? Because otherwise it'd run LoaderThread() on destructed member variables
	// This issue occred when closing the exe while still loading/streaming resources
	// myThreadPool.StopJoinAll();

	std::cout << "~ResourceManager" << std::endl;

	for (auto& t : myThreads)
	{
		LoaderThread.myResourcesToLoaderCv.notify_all();

		if (t.joinable())
		{
			t.join();
		}
	}

	std::cout << "~ResourceManager All joined" << std::endl;
}

bool Engine::ResourceManager::Init(
	GraphicsEngine& aGraphicsEngine, FileWatcher* aFileWatcher)
{
	myGraphicsEngine = &aGraphicsEngine;

	//{
	//	std::unique_lock<std::mutex> lock(LoaderThread.myResourcesToLoadMutex);

	//	for (int i = 0; i < 20; ++i)
	//	{
	//		LoaderThread.myResourcesToLoad.push_back(MakeOwned<std::vector<Owned<ResourceBase>>>());
	//	}
	//}

// 	{
// 		std::unique_lock<std::mutex> lock(LoaderThread.myLoadedResourcesMutex);
// 
// 		for (int i = 0; i < 20; ++i)
// 		{
// 			LoaderThread.myLoadedResources.push_back(
// 				MakeOwned<std::unordered_map<std::string, Owned<ResourceBase>>>());
// 		}
// 	}

	//{
	//	std::unique_lock<std::mutex> lock(myShare.myGlobalResourceMutex);

	//	for (int i = 0; i < 20; ++i)
	//	{
	//		myShare.myResources.push_back(
	//			MakeOwned<std::unordered_map<std::string, Shared<ResourceBase>>>());
	//	}
	//}

	// {
	// 	std::unique_lock<std::mutex> lock(myShare.myGlobalResourcesToLoadMutex);
	// 
	// 	for (int i = 0; i < 20; ++i)
	// 	{
	// 		// RenderThread.myResources.push_back(
	// 		// 	MakeOwned<std::unordered_map<std::string, Shared<ResourceBase>>>());
	// 
	// 		myShare.myResourcesToLoad.push_back(MakeOwned<std::vector<Owned<ResourceBase>>>());
	// 	}
	// }

	myFileWatcher = aFileWatcher;

	// TEMP, do not create this for just to load placeholder, bad
	auto textureLoader = MakeOwned<TextureLoader>();

	if (!textureLoader->Init(aGraphicsEngine.GetDxFramework().GetDevice(),
		*aGraphicsEngine.GetDxFramework().GetContext()))
	{
		return false;
	}

	// Load the placeholder texture
	myPlaceholderTexture = textureLoader->LoadTexture(
		GetEngine().GetPaths().GetPathString(PathEnum::PlaceholderTexture));

	if (myPlaceholderTexture == nullptr)
	{
		LOG_ERROR(LogType::Renderer) << "Failed to load placeholder texture";
		return false;
	}

	int totalThreads = std::thread::hardware_concurrency();
	totalThreads = std::max(1, totalThreads);

	// The thread that we call this function from Init()
	// is considered the main thread
	// and we must create a thread context for that as well
	// TODO: MEMORY LEAK

	CreateThreadContext(std::this_thread::get_id());

	for (int i = 0; i < totalThreads; ++i)
	{
		ResourceThreadData* threadData = DBG_NEW ResourceThreadData();

		if (!threadData->Init(aGraphicsEngine.GetDxFramework().GetDevice(),
			*aGraphicsEngine.GetDxFramework().GetContext(),
			*this))
		{
			LOG_ERROR(LogType::Renderer) << "Failed to init resource thread data";
			return false;
		}

		auto t = std::thread(&ResourceManager::LoadingThread, this, threadData);

		SetThreadDescription(t.native_handle(), L"LoadingThread");

		if (!CreateThreadContext(t.get_id()))
		{
			return false;
		}

		/*
		// TODO: MEMORY LEAK
		ResourceThreadContext* rsm = DBG_NEW ResourceThreadContext(myResourceManagerSettings);

		if (!rsm->Init(aGraphicsEngine.GetDxFramework().GetDevice(),
			*aGraphicsEngine.GetDxFramework().GetContext(),
			*this))
		{
			return false;
		}

		myContexts[t.get_id()] = rsm;
		*/

		myThreads.push_back(std::move(t));
	}

	return true;
}

void Engine::ResourceManager::SetIsResourceLoadingAllowed(const bool aIsAllowed)
{
	myIsResourceLoadingAllowed = aIsAllowed;
}

void Engine::ResourceManager::TransferResourceToRenderThread()
{
	ZoneScopedN("TransferResourceToRenderThread");

	{
		// Hopefully no deadlock can occur here
		std::lock_guard<std::mutex> lock(LoaderThread.myLoadedResourcesMutex);
		std::unique_lock<std::mutex> lock2(myShare.myGlobalResourceMutex);

		std::vector<std::pair<int, std::string>> loaderResourcesToRemove;

		for (int resourceType = 0; resourceType < LoaderThread.myLoadedResources.size();
			++resourceType)
		{
			auto& loadedResources = LoaderThread.myLoadedResources[resourceType];

			// Move from loading thread to render thread
			// now render thread can access it without any locks :D
			for (auto& [resIdentifier, resource] : loadedResources)
			{
				assert(myShare.myResources[resource->GetTypeID()].find(resIdentifier)
					!= myShare.myResources[resource->GetTypeID()].end()
					&& "must exist");

				auto& renderThreadResource
					= myShare.myResources[resource->GetTypeID()][resIdentifier];

				if (renderThreadResource->myState == ResourceState::Loaded)
				{
					LOG_WARNING(LogType::Resource) <<
						"Resource has already been loaded once, fix in future (filip): " << resIdentifier;
				}

				// assert(renderThreadResource->myState != ResourceState::Loaded
				// 	&& "if this is valid, means it has already been loaded once");

				// Transfer ownership of the resource to the render thread
				renderThreadResource->Swap(resource.get());
				renderThreadResource->myLoadResult = resource->myLoadResult;
				renderThreadResource->myState = resource->myState;
				// Must copy the children as well because they were most likely added when calling
				// Create() which occured on the loading thread, and we must sync that to the main
				// thread resource
				renderThreadResource->myChildren = resource->GetChildren(false);
				auto& mainThreadResourceThreadData
					= myContexts[std::this_thread::get_id()]->myResourceThreadData;
				renderThreadResource->PostCreation(mainThreadResourceThreadData);

				// Add it to be removed from loader thread
				loaderResourcesToRemove.push_back(std::make_pair(resourceType, resIdentifier));
			}
		}

		// Remove the transferred resources from the loading thread
		for (const auto& [resourceType, pathToRemove] : loaderResourcesToRemove)
		{
			LoaderThread.myLoadedResources[resourceType].erase(pathToRemove);
		}
	}

	{
		/*
			Transfer the ownership of what resources to load on a sync point
			Why? Because it avoids us having to lock the mutex that is being used
			by the loader threads every time we call RequestLoading() on a resource

			TL;DR It improves performance hopefully
		*/

		// Hopefully no deadlock here
		std::lock_guard<std::mutex> lock(LoaderThread.myResourcesToLoadMutex);
		std::unique_lock<std::mutex> lock2(myShare.myGlobalResourcesToLoadMutex);

		for (int resourceType = 0; resourceType < myShare.myResourcesToLoad.size(); ++resourceType)
		{
			for (auto& resourceToLoad : myShare.myResourcesToLoad[resourceType])
			{
				assert(resourceType < LoaderThread.myResourcesToLoad.size()
					&& "not enough resource type space, preload more");

				LoaderThread.myResourcesToLoad[resourceType].push_back(std::move(resourceToLoad));
			}

			myShare.myResourcesToLoad[resourceType].clear();
		}

		// Update the value if all resources are loaded
		// this is used to display a loading screen while everything is loading
		myIsAllResourcesLoaded = true;

		for (const auto& lol : LoaderThread.myResourcesToLoad)
		{
			if (!lol.empty())
			{
				myIsAllResourcesLoaded = false;
				break;
			}
		}

		// Check if any resources we're asked for this frame
		for (const auto& lol : myShare.myResourcesToLoad)
		{
			if (!lol.empty())
			{
				myIsAllResourcesLoaded = false;
				break;
			}
		}

		// Must have moved all resources to shared render thread for
		// it to be considered finished
		for (const auto& lol : LoaderThread.myLoadedResources)
		{
			if (!lol.empty())
			{
				myIsAllResourcesLoaded = false;
				break;
			}
		}

		/*
		if (myIsAllResourcesLoaded)
		{
			int test = 0;
		}
		*/
	}

	for (auto& [lol, lol2] : myContexts)
	{
		lol2->TransferResourceToRenderThread(LoaderThread, myShare);
	}

	// Unlock the mutex above before notifying, otherwise
	// it would simply block immediately
	LoaderThread.myResourcesToLoaderCv.notify_all();
}

void Engine::ResourceManager::LoadingThread(ResourceThreadData* aThreadData)
{
	// tracy::SetThreadName("LoadingThread");

	// Only reason for this is to delete it
	std::unique_ptr<ResourceThreadData> temp(aThreadData);

	while (myRunning)
	{
		// FrameMarkStart("ldrthread");

		ZoneScopedN("LoadingThread");

		Owned<ResourceBase> resourceToLoad = nullptr;

		std::vector<ResourceRef<ResourceBase>> children;

		// Wait until there is a resource to load
		{
			std::unique_lock<std::mutex> lock(LoaderThread.myResourcesToLoadMutex);
			LoaderThread.myResourcesToLoaderCv.wait(lock,
				[this]()
				{
					if (!myRunning)
					{
						// We want to stop the thread
						return true;
					}

					for (const auto& resources : LoaderThread.myResourcesToLoad)
					{
						if (!resources.empty())
						{
							// Wake up
							return true;
						}
					}

					// Continue waiting
					return false;
				});

			if (!myRunning)
			{
				return;
			}

			for (auto& resources : LoaderThread.myResourcesToLoad)
			{
				if (!resources.empty())
				{
					// Take the first resource in the queue
					resourceToLoad = std::move(resources.front());
					resources.erase(resources.begin());
					break;
				}
			}
		}

		// Sleep(500);

		// std::cout << "RESOURCEMANAGER: LOADING " << resourceToLoad->GetPath() << " TID: " << GetCurrentThreadId() << std::endl;

		const bool result = resourceToLoad->Create(*aThreadData);

		if (result)
		{
			// non recursive, because it will become recursive automatically because we call RequestLoading()
			// on the children at end of function
			children = resourceToLoad->GetChildren(false);

			LOG_INFO(LogType::Resource)
				<< "Loaded (" << std::this_thread::get_id() << ") " << resourceToLoad->GetPath();
		}
		else
		{
			LOG_ERROR(LogType::Resource) << "Failed load (" << std::this_thread::get_id() << ") "
				<< resourceToLoad->GetPath();
		}

		resourceToLoad->myLoadResult
			= result == true ? ResourceLoadResult::Succeeded : ResourceLoadResult::Failed;
		resourceToLoad->myState = ResourceState::Loaded;

		{
			std::lock_guard<std::mutex> lock(LoaderThread.myLoadedResourcesMutex);

			assert(resourceToLoad->GetTypeID() < LoaderThread.myLoadedResources.size()
				&& "not enough resource type space, preload more");

			assert(resourceToLoad->GetType() != ResourceType::Unknown);

			// Save it to later on be transferred to the render thread
			LoaderThread.myLoadedResources[resourceToLoad->GetTypeID()][resourceToLoad->GetId()]
				= std::move(resourceToLoad);
		}

		// Request the children
		for (int i = 0; i < children.size(); ++i)
		{
			children[i]->RequestLoading();
		}

		// FrameMarkStart("ldrthreadend");
	}
}

FileWatcher* Engine::ResourceManager::GetFileWatcher()
{
	return myFileWatcher;
}

Engine::GraphicsEngine& Engine::ResourceManager::GetGraphicsEngine()
{
	return *myGraphicsEngine;
}

const Engine::ResourceManagerSettings& Engine::ResourceManager::GetSettings() const
{
	return myResourceManagerSettings;
}

bool Engine::ResourceManager::IsAllResourcesLoaded() const
{
	return myIsAllResourcesLoaded;
}

bool Engine::ResourceManager::CreateThreadContext(const std::thread::id aThreadId)
{
	myContexts[aThreadId] = DBG_NEW ResourceThreadContext(myResourceManagerSettings);

	if (!myContexts[aThreadId]->Init(myGraphicsEngine->GetDxFramework().GetDevice(),
		*myGraphicsEngine->GetDxFramework().GetContext(),
		*this))
	{
		return false;
	}

	return true;
}

Shared<Engine::Texture2D> Engine::ResourceManager::GetPlaceholderTexture()
{
	return myPlaceholderTexture;
}
