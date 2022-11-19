#pragma once

#include "ResourceId.h"
#include "ResourceEnums.h"

#include <vector>

namespace Engine
{
	class ResourceThreadData;
	class ResourceManager;
	class ResourceObserver;

	template <typename T>
	class ResourceRef;

	class ResourceBase
	{
	public:
		static inline ResourceType ourResourceType = ResourceType::Unknown;

		ResourceBase(ResourceManager& aResourceManager,
			const std::string& aPath,
			const ResourceId& aId);

		// When creating a raw manual resource, we do not need resource manager or the path
		ResourceBase();

		virtual ~ResourceBase();
		ResourceBase(const ResourceBase&) = delete;

		const std::string& GetPath() const;
		const ResourceId& GetId() const;

		void RequestLoading() const;
		void Load() const;
		void Unload();

		bool IsValid() const;
		ResourceState GetState() const;

		// Recursively checks if children are loaded as well
		bool IsLoaded() const;

		void AddObserver(ResourceObserver* aObserver);
		void RemoveObserver(ResourceObserver* aObserver);

		void AddRef();
		void RemoveRef();

		void AddChild(const ResourceRef<ResourceBase>& aResource);

		std::vector<ResourceRef<ResourceBase>> GetChildren(const bool aRecursive = false) const;

		ResourceType GetType() const;
		int GetTypeID() const;

		uint16_t GetID() const;

		size_t GetIdHash() const;

	protected:
		virtual bool Create(ResourceThreadData& aThreadData) { return false; }
		virtual void PostCreation(ResourceThreadData& aResourceThreadData)
		{
			aResourceThreadData;
		}

		virtual void Swap(ResourceBase* aResourceBase) {}
		virtual void RequestMe() const {}
		virtual void LoadMe() const {}
		virtual void UnloadMe() {}

		friend class ResourceManager;

		std::string myPath;
		ResourceId myIdentifier;

		ResourceManager* myResourceManager = nullptr;

		ResourceLoadResult myLoadResult = ResourceLoadResult::None;

		// NOTE: current only used manually in one resource: AnimationResource
		// not a proper system ready for use
		std::vector<ResourceObserver*> myObservers;

		// A mutex is needed if we add a mutex from another thread
		// which occurs when we create a material that adds itself to observer of its
		// vertex/pixel shaders.
		// I dont like it, but it has to do for now.
		std::mutex myObserversMutex;

		mutable std::mutex myChildrenMutex;
		std::vector<ResourceRef<ResourceBase>> myChildren;

		mutable ResourceState myState = ResourceState::Unloaded;

	private:
		void GetChildrenInternal(
			const ResourceBase& aResBase,
			std::vector<ResourceRef<ResourceBase>>& aResources) const;

		void IsLoadedInternal(const ResourceBase& aResBase, bool& aIsLoaded) const;

		// static void LoadChildrenRecursively(const ResourceBase& aResBase);
		// static void RequestChildrenRecursively(const ResourceBase& aResBase);

	private:

		//mutable bool myLoadRequested = false;
		// bool myLoadFinished = false;
		// int myResourceId = -1;

		ResourceType myType = ResourceType::Unknown;

		// A unique ID generate for this type of resource
		uint16_t myID = -1;

		//int myReferenceCount = 0;
		std::atomic_int myReferenceCount = 0;

		size_t myIdHash = 0;
	};
}