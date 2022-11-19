#pragma once

#include "../ResourceManager.h"
#include "../ResourceId.h"
#include "../ResourceBase.h"

namespace Engine
{
	class GameObjectPrefab;

	class GameObjectPrefabResource : public ResourceBase
	{
	public:
		static inline ResourceType ourResourceType = ResourceType::GameObjectPrefab;

	public:
		GameObjectPrefabResource(
			ResourceManager& aResourceManager,
			const std::string& aPath,
			const ResourceId& aId);

		virtual ~GameObjectPrefabResource();

		GameObjectPrefabResource(const GameObjectPrefabResource& aOther);

		GameObjectPrefab& Get() const;
		GameObjectPrefab& Get() { return *myGameObjectPrefab; }

		void SetLoadAssociatedResources(const bool aLoadAssociatedResources);

	protected:
		friend class ResourceManager;

		bool Create(ResourceThreadData& aThreadData) override;
		void Swap(ResourceBase* aResourceBase);
		void PostCreation(ResourceThreadData& aResourceThreadData) override;
		void RequestMe() const;
		void LoadMe() const;
		void UnloadMe();

	private:
		Owned<GameObjectPrefab> myGameObjectPrefab = nullptr;

		bool myLoadAssociatedResources = false;
	};
}