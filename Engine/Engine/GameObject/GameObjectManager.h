#pragma once

#include "ComponentSystem/ComponentSystem.h"
#include "Engine/EngineOrder.h"
#include "SingletonComponent.h"
#include "Common/Random.h"
#include "Engine/ResourceManagement/ResourceRef.h"

class GameObject;
class TimeStamp;

namespace Engine
{
	class Scene;
	class ImguiReflector;
	class ComponentSystem;

	class GameObjectManager
	{
	public:
		GameObjectManager();
		~GameObjectManager();
		GameObjectManager(const GameObjectManager& aOther);

		bool Init(Scene& aScene);

		void Update(const float aDeltaTime);
		void UpdateSystems(eEngineOrder aOrder);
		void EditorUpdate();
		void Render() const;

		// Creates the gameobjects that was "added to be created"
		void CreateQueuedGameObjects();

		const std::vector<Shared<GameObject>>& GetGameObjects() const;

		// Low performance, only used in the editor for specific cases that are non-performance needed
		// Returns -1 if not found
		int GetGameObjectIndex(const GameObject& aGameObject) const;
		Shared<GameObject> GetGameObjectFromIndex(const int aIndex);

		GameObject* FindGameObject(int aUUID);
		GameObject* FindGameObject(const std::string& aName) const;
		const Shared<GameObject>& FindGameObjectShared(const int aUUID);

		void RemoveDestroyedGameObjects();


		template <class T, class... Args>
		T* AddGameObject(Args&&... args);

		// Creates, but does not add it to the scene
		Shared<GameObject> CreateGameObject();

		// Shared<GameObject> AddGameObjectForEditor2();

		// NOTE(filip): the reason the name is weird is because the other function with the same
		// name actually has the incorrect name, the other one should be called CreateGameObject()
		void AddGameObject2(Shared<GameObject> aGameObject);

		// Removes the gameobject, but does not delete it from memory
		// intended use it for the editor when calling delete on a gameobject from the scene graph
		void RemoveGameObject(const GameObject* aGameObject);

		ComponentSystem& GetComponentSystem();

		// Find the gameobject and gives back a shared version of it
		// the reason is when iterating the children of a GameObject
		// I need them as Shared to keep them alive in the editor
		Shared<GameObject> ToShared(GameObject* aGameObject);

		// Glorified copy constructor but gives the scene as well which is needed
		void CloneFromAnother(Scene& aScene, const GameObjectManager& aOther);

		void ReflectGameObjects(Reflector& aReflector);
		void ReflectAllGameObjectComponents(Reflector& aReflector);

		void MoveGameObjectToIndex(const Shared<GameObject>& aDroppedGameObject, const int aTargetIndex);

		// Updates all gameobjects that are of the same prefab instance
		void UpdatePrefabInstances(GameObject* aBeforeChangedPrefabGameObject, const GameObjectPrefabRef& aGameObjectPrefabResource);

	private:
		friend class SceneSerializerJson;
		friend class AddGameObjectCommand;
		friend class GameObjectPrefab;
		friend class RemoveGameObjectCommand;
		friend class GameObject;
		friend class Scene;
		friend class InspectableGameObjectPrefab;
		friend class ImguiReflector;

	private:
		Scene* myScene = nullptr;
		ComponentSystem myComponentSystem;

		// Max 100 singleton components
		std::vector<SingletonComponentEntry> mySingletonComponentEntries;

		std::vector<Shared<GameObject>> myGameObjects;
		std::vector<Shared<GameObject>> myGameObjectsToAdd;
	};

	template <class T, class... Args>
	T* GameObjectManager::AddGameObject(Args&&... args)
	{
		Shared<T> gameObject = std::make_shared<T>(std::forward<Args&&>(args)...);

		gameObject->InitTransform(gameObject);

		// gameObject->myScene = myScene;
		gameObject->myWeakPtr = gameObject;
		// TODO: Fix a better system for UUIDs
		//gameObject->myUUID = ourGameObjectIndex++;
		// gameObject->myUUID = Random::RandomInt(INT_MIN, INT_MAX);

		gameObject->myScene = myScene;

		myGameObjectsToAdd.emplace_back(gameObject);

		return static_cast<T*>(gameObject.get());
	}
}