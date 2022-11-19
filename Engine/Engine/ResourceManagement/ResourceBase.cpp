#include "pch.h"
#include "ResourceBase.h"
#include "ResourceThreadContext.h"

namespace Engine
{
	Engine::ResourceBase::ResourceBase(ResourceManager& aResourceManager,
		const std::string& aPath,
		const ResourceId& aId)
		: myPath(aPath)
		, myIdentifier(aId)
		, myResourceManager(&aResourceManager)
	{
		std::hash<std::string> hasher;
		myIdHash = hasher(aId);
	}

	ResourceBase::~ResourceBase()
	{
	}

	ResourceBase::ResourceBase()
	{
	}

	const std::string& Engine::ResourceBase::GetPath() const
	{
		return myPath;
	}

	const Engine::ResourceId& ResourceBase::GetId() const
	{
		return myIdentifier;
	}

	void ResourceBase::RequestLoading() const
	{
		if (myState == ResourceState::Requested || myState == ResourceState::Loaded)
		{
			return;
		}

		RequestMe();

		// RequestChildrenRecursively(*this);

		myState = ResourceState::Requested;
	}

	void ResourceBase::Load() const
	{
		if (myState == ResourceState::Loaded)
		{
			return;
		}

		LoadMe();

		// Load the children recursively as well
		// LoadChildrenRecursively(*this);

		myState = ResourceState::Loaded;
	}

	void ResourceBase::Unload()
	{
		if (myState == ResourceState::Unloaded)
		{
			return;
		}

		UnloadMe();

		LOG_INFO(LogType::Resource) << "Unloaded " << myPath;

		myState = ResourceState::Unloaded;
		myLoadResult = ResourceLoadResult::None;
	}

	bool ResourceBase::IsValid() const
	{
		return myState == ResourceState::Loaded
			&& myLoadResult == ResourceLoadResult::Succeeded;
	}

	ResourceState ResourceBase::GetState() const
	{
		return myState;
	}

	bool ResourceBase::IsLoaded() const
	{
		bool isLoaded = true;

		IsLoadedInternal(*this, isLoaded);

		return isLoaded;
	}

	void ResourceBase::AddObserver(ResourceObserver* aObserver)
	{
		std::unique_lock lock(myObserversMutex);

		auto result = std::find(myObservers.begin(), myObservers.end(), aObserver);

		if (result == myObservers.end())
		{
			myObservers.push_back(aObserver);
		}
	}

	void ResourceBase::RemoveObserver(ResourceObserver* aObserver)
	{
		std::unique_lock lock(myObserversMutex);

		auto result = std::find(myObservers.begin(), myObservers.end(), aObserver);

		if (result != myObservers.end())
		{
			myObservers.erase(result);
		}
	}

	void ResourceBase::AddRef()
	{
		++myReferenceCount;
	}

	void ResourceBase::RemoveRef()
	{
		--myReferenceCount;

		// assert(myReferenceCount >= 0);

		if (myReferenceCount == 0)
		{
			// Unload();
		}
	}

	void ResourceBase::AddChild(const ResourceRef<ResourceBase>& aResource)
	{
		std::scoped_lock lock(myChildrenMutex);

		myChildren.push_back(aResource);
	}

	std::vector<Engine::ResourceRef<ResourceBase>> ResourceBase::GetChildren(const bool aRecursive) const
	{
		std::vector<ResourceRef<ResourceBase>> result;

		GetChildrenInternal(*this, result);

		return result;
	}

	ResourceType ResourceBase::GetType() const
	{
		return myType;
	}

	int ResourceBase::GetTypeID() const
	{
		return static_cast<int>(myType);
	}

	uint16_t ResourceBase::GetID() const
	{
		return myID;
	}

	size_t ResourceBase::GetIdHash() const
	{
		return myIdHash;
	}

	void ResourceBase::GetChildrenInternal(
		const ResourceBase& aResBase,
		std::vector<ResourceRef<ResourceBase>>& aResources) const
	{
		{
			std::scoped_lock lock(aResBase.myChildrenMutex);
			aResources.insert(aResources.begin(), aResBase.myChildren.begin(), aResBase.myChildren.end());

			for (const auto& child : aResBase.myChildren)
			{
				GetChildrenInternal(*child, aResources);
			}
		}
	}

	void ResourceBase::IsLoadedInternal(const ResourceBase& aResBase, bool& aIsLoaded) const
	{
		if (myState != ResourceState::Loaded)
		{
			aIsLoaded = false;
			return;
		}

		for (const auto& child : aResBase.myChildren)
		{
			IsLoadedInternal(*child, aIsLoaded);
		}
	}

	//void ResourceBase::LoadChildrenRecursively(const ResourceBase& aResBase)
	//{
	//	std::scoped_lock lock(aResBase.myChildrenMutex);
	//	for (const auto& child : aResBase.myChildren)
	//	{
	//		child->Load();
	//		// LoadChildrenRecursively(*child);
	//	}
	//}

	//void ResourceBase::RequestChildrenRecursively(const ResourceBase& aResBase)
	//{
	//	std::scoped_lock lock(aResBase.myChildrenMutex);
	//	for (const auto& child : aResBase.myChildren)
	//	{
	//		child->RequestLoading();
	//		// RequestChildrenRecursively(*child);
	//	}
	//}
}