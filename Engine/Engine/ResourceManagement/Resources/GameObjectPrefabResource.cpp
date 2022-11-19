#include "pch.h"
#include "GameObjectPrefabResource.h"
#include "../ResourceThreadContext.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"
#include "Engine/Reflection/ResourceRefReflector.h"
#include "Engine/GameObject/GameObject.h"

Engine::GameObjectPrefabResource::GameObjectPrefabResource(
	ResourceManager& aResourceManager,
	const std::string& aPath,
	const ResourceId& aId)
	: ResourceBase(aResourceManager, aPath, aId)
{
}

Engine::GameObjectPrefabResource::~GameObjectPrefabResource()
{
}

Engine::GameObjectPrefabResource::GameObjectPrefabResource(const GameObjectPrefabResource& aOther) :
	ResourceBase(*aOther.myResourceManager, aOther.myPath, aOther.myIdentifier),
	myLoadAssociatedResources(aOther.myLoadAssociatedResources)
{
	assert(aOther.myGameObjectPrefab == nullptr && "why is this not nullptr? it is "
		"supposed to be because we only ever copy this in ResourceManager "
		"before it has been fully created");
}

Engine::GameObjectPrefab& Engine::GameObjectPrefabResource::Get() const
{
	return *myGameObjectPrefab;
}

void Engine::GameObjectPrefabResource::SetLoadAssociatedResources(const bool aLoadAssociatedResources)
{
	myLoadAssociatedResources = aLoadAssociatedResources;
}

bool Engine::GameObjectPrefabResource::Create(ResourceThreadData& aThreadData)
{
	auto gameObjectPrefab = MakeOwned<GameObjectPrefab>();

	if (!gameObjectPrefab->Deserialize(myPath, myLoadAssociatedResources))
	{
		return false;
	}

	// Must add children resources to child to ensure waiting for them
	// to be loaded
	ResourceRefReflector resRefRefl;
	gameObjectPrefab->GetGameObject()->ReflectChildrenAndComponents(resRefRefl);

	const auto& resRefs = resRefRefl.GetResourceRefs();

	// must make them child
	for (const auto& ref : resRefs.GetResources())
	{
		AddChild(ref);
	}

	myGameObjectPrefab = std::move(gameObjectPrefab);

	return true;
}

void Engine::GameObjectPrefabResource::Swap(ResourceBase* aResourceBase)
{
	// We know for a fact that aResourceBase input argument is of the same class
	// otherwise something went wrong in the ResourceManager
	auto other = reinterpret_cast<GameObjectPrefabResource*>(aResourceBase);

	myGameObjectPrefab.swap(other->myGameObjectPrefab);
	myLoadAssociatedResources = other->myLoadAssociatedResources;
}

void Engine::GameObjectPrefabResource::PostCreation(ResourceThreadData& aResourceThreadData)
{
	// NOTE(filip): we cannot watch the file for changes because we use raw pointers to the prefab gameobject
	// and if we reallocate the value, does pointers will get invalidated
	// 
	//myResourceManager->GetFileWatcher()->WatchFile(myPath,
	//	[this, &aResourceThreadData](const std::string& aPath)
	//	{
	//		// this lambda will be called from render thread (main thread)
	//		// so its ok to edit stuff here because renderer 
	//		// won't touch the sprite during this time
	//		Create(aResourceThreadData);
	//	});
}

void Engine::GameObjectPrefabResource::RequestMe() const
{
	// Sadly this is required to avoid having the ResourceBase::RequestLoading as a NON-template function
	// Why do we want that as a non-template function? To avoid circular include with templated shit..
	// ABSOLUTE DOG
	myResourceManager->RequestLoading<GameObjectPrefabResource>(myIdentifier);
}

void Engine::GameObjectPrefabResource::LoadMe() const
{
	myResourceManager->Load<GameObjectPrefabResource>(myIdentifier);
}

void Engine::GameObjectPrefabResource::UnloadMe()
{
	myGameObjectPrefab = nullptr;
}
