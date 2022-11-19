#include "pch.h"
#include "GameObjectPrefabInstance.h"
//#include "Engine/ResourceManagement/Resources/GameObjectPrefabResource.h"
//
//const GameObjectPrefabRef& Engine::GameObjectPrefabInstance::GetPrefab() const
//{
//	return myPrefab;
//}
//
//void Engine::GameObjectPrefabInstance::AddExistingComponent(Component* aComponent)
//{
//	GameObject::AddExistingComponent(aComponent);
//
//	// If we modified the master prefab, do it for all the instances in the scene
//	if (IsMasterPrefab())
//	{
//
//	}
//}
//
//void Engine::GameObjectPrefabInstance::RemoveComponentNoFree(Component* aComponent)
//{
//	GameObject::RemoveComponentNoFree(aComponent);
//
//	// NOTE: this won't be needed, we can always to this in the inspect code
//	// just like we do when we will draw the gameobjec/components different if they
//	// are overriden
//
//	// Means we can remove this class and simply use the GameObject as a class
//
//	//// If we modified the master prefab, do it for all the instances in the scene
//	//if (IsMasterPrefab())
//	//{
//	//	auto& gameObjects = myScene->GetGameObjects();
//
//	//	for (auto& gameObject : gameObjects)
//	//	{
//	//		if (!gameObject->IsPrefabInstance())
//	//		{
//	//			continue;
//	//		}
//
//	//		GameObjectPrefabInstance* prefabInstance =
//	//			static_cast<GameObjectPrefabInstance*>(gameObject.get());
//
//	//		const auto& prefab = prefabInstance->GetPrefab();
//
//	//		if (myPrefab != prefab)
//	//		{
//	//			continue;
//	//		}
//
//	//		assert(prefab && prefab->IsValid());
//
//	//		gameObject->AddExistingComponent();
//
//	//		// When reflecting on each Component & GameObject we assume
//	//		// they their Reflect() layout is exactly the same 1:1
//
//	//		// First we Reflect on both GameObject's
//	//		// GameObjectPrefabInstance* prefabGameObject = myGameObjectPrefab->Get().myGameObject.get();
//	//	}
//	//}
//}
//
//bool Engine::GameObjectPrefabInstance::IsMasterPrefab() const
//{
//	// We assume that its a master prefab, if this is nullptr, 
//	// because it is not allowed to be valid for the master prefab because then a circular
//	return !myPrefab;
//}
