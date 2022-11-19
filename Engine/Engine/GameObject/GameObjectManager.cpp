#include "pch.h"
#include "GameObjectManager.h"
#include "GameObject.h"
#include "Engine/Reflection/Reflector.h"
#include "Engine/ResourceManagement/Resources/GameObjectPrefabResource.h"
#include "Prefab/GameObjectPrefab.h"

Engine::GameObjectManager::GameObjectManager() = default;

Engine::GameObjectManager::~GameObjectManager()
{
	// We must destroy the gameobjects in the order of children first, then parent
	// Otherwise we'll crash in Transform destructor because it tries to remove

	// Must release the transforms first, otherwise it tries to access destroyed parent
	for (auto& g : myGameObjects)
	{
		g->GetTransform().ReleaseTransform();
	}
	for (auto& g : myGameObjectsToAdd)
	{
		g->GetTransform().ReleaseTransform();
	}

	myGameObjects.clear();
	myGameObjectsToAdd.clear();
}

Engine::GameObjectManager::GameObjectManager(const GameObjectManager & aOther)
{
	// myScene = aOther.myScene;

	assert(false);

	//myComponentSystem = aOther.myComponentSystem;

	//// clear all of the component pointers
	//// why? because we add them ourselves in UpdateInternalReferences()
	//for (auto& handler : myComponentSystem.myComponentHandlers)
	//{
	//	handler.myActiveComponents.clear();
	//}

	//for (Shared<GameObject> g : aOther.myGameObjects)
	//{
	//	// Must be top level gameobject, since we recursively clone the children
	//	if (g->GetTransform().GetParent() != nullptr)
	//	{
	//		continue;
	//	}

	//	//CloneGameObject(myGameObjects, *g, nullptr);
	//	g->Clone(*this);
	//}

	//for (Shared<GameObject> g : aOther.myGameObjectsToAdd)
	//{
	//	// Must be top level gameobject, since we recursively clone the children
	//	if (g->GetTransform().GetParent() != nullptr)
	//	{
	//		continue;
	//	}

	//	g->Clone(*this);
	//	// CloneGameObject(myGameObjectsToAdd, *g, nullptr);
	//}
}

bool Engine::GameObjectManager::Init(Scene & aScene)
{
	myScene = &aScene;
	myComponentSystem.Init();

	return true;
}

void Engine::GameObjectManager::Update(const float aDeltaTime)
{
	ZoneNamedN(scope1, "GameObjectManager::Update", true);

	for (auto& g : mySingletonComponentEntries)
	{
		if (g.myGameObject)
		{
			if (!g.myGameObject->myHasAwaked)
			{
				g.myGameObject->InternalAwake();
			}
		}
	}

	for (int i = 0; i < myGameObjects.size(); i++)
	{
		auto& g = myGameObjects[i];

		if (!g->myHasAwaked && g->IsActive())
		{
			g->InternalAwake();
		}
	}

	for (auto& g : mySingletonComponentEntries)
	{
		if (g.myGameObject)
		{
			if (!g.myGameObject->myHasStarted)
			{
				g.myGameObject->InternalStart();
			}
		}
	}

	for (int i = 0; i < myGameObjects.size(); i++)
	{
		auto& g = myGameObjects[i];

		if (!g->myHasStarted && g->IsActive())
		{
			g->InternalStart();
		}

		if (g->IsDestroyed())
		{
			continue;
		}

		if (g != nullptr && g->IsActive())
		{
			g->Update(aDeltaTime);
		}
	}

	RemoveDestroyedGameObjects();
}

void Engine::GameObjectManager::UpdateSystems(eEngineOrder aOrder)
{
	myComponentSystem.Execute(aOrder);
}

void Engine::GameObjectManager::EditorUpdate()
{
	myComponentSystem.EditorExecute();

	RemoveDestroyedGameObjects();
}

void Engine::GameObjectManager::Render() const
{
	myComponentSystem.Render();
}

void Engine::GameObjectManager::CreateQueuedGameObjects()
{
	ZoneScopedN("GameObjectManager::CreateQueuedGameObjects");

	for (int i = 0; i < myGameObjectsToAdd.size(); i++)
	{
		myGameObjects.emplace_back(myGameObjectsToAdd[i]);
		// myGameObjectsToAdd[i]->InternalStart();
	}

	myGameObjectsToAdd.clear();
}

Shared<GameObject> Engine::GameObjectManager::CreateGameObject()
{
	Shared<GameObject> gameObject = MakeShared<GameObject>();

	gameObject->InitTransform(gameObject);

	gameObject->myWeakPtr = gameObject;
	// gameObject->myUUID = ourGameObjectIndex++;

	return gameObject;
}

void Engine::GameObjectManager::AddGameObject2(Shared<GameObject> aGameObject)
{
	myGameObjectsToAdd.push_back(aGameObject);
}

void Engine::GameObjectManager::RemoveGameObject(const GameObject * aGameObject)
{
	{
		const auto findResult = std::find_if(myGameObjects.begin(),
			myGameObjects.end(),
			[aGameObject](const Shared<GameObject>& aG)
			{
				return aG.get() == aGameObject;
			});

		if (findResult != myGameObjects.end())
		{
			myGameObjects.erase(findResult);
			return;
		}
	}

	{
		const auto findResult = std::find_if(myGameObjectsToAdd.begin(),
			myGameObjectsToAdd.end(),
			[aGameObject](const Shared<GameObject>& aG)
			{
				return aG.get() == aGameObject;
			});

		if (findResult != myGameObjectsToAdd.end())
		{
			myGameObjectsToAdd.erase(findResult);
			return;
		}
	}
}

Engine::ComponentSystem& Engine::GameObjectManager::GetComponentSystem()
{
	return myComponentSystem;
}

Shared<GameObject> Engine::GameObjectManager::ToShared(GameObject * aGameObject)
{
	for (int i = 0; i < myGameObjectsToAdd.size(); i++)
	{
		if (myGameObjectsToAdd[i].get() == aGameObject)
		{
			return myGameObjectsToAdd[i];
		}
	}

	for (int i = 0; i < myGameObjects.size(); i++)
	{
		if (myGameObjects[i].get() == aGameObject)
		{
			return myGameObjects[i];
		}
	}

	assert(false);

	return nullptr;
}

void Engine::GameObjectManager::CloneFromAnother(Scene & aScene, const GameObjectManager & aOther)
{
	myScene = &aScene;

	myComponentSystem = aOther.myComponentSystem;

	// clear all of the component pointers
	// why? because we add them ourselves when cloning the gameobject
	for (auto& handler : myComponentSystem.myComponentHandlers)
	{
		handler.myActiveComponents.clear();
	}

	// WILLIAM HADE CRASH HÄR, FÖR ATT LANDSPACE.
	// LÖSNINGEN VAR ITERERA BAKLÄNGES
	// PROBLEMET MED DET VAR ATT DÅ REVERSEADE ALLA GAMEOBJECT I SCENE HIERARKIN VARJE GÅNG.

	for (const Shared<GameObject>& g : aOther.myGameObjects)
	{
		// Must be top level gameobject, since we recursively clone the children
		if (g->GetTransform().GetParent() != nullptr)
		{
			continue;
		}

		g->Clone(*this, false);
	}

	for (const Shared<GameObject>& g : aOther.myGameObjectsToAdd)
	{
		// Must be top level gameobject, since we recursively clone the children
		if (g->GetTransform().GetParent() != nullptr)
		{
			continue;
		}

		g->Clone(*this, false);
	}
}

void Engine::GameObjectManager::ReflectGameObjects(Reflector & aReflector)
{
	for (auto& g : myGameObjects)
	{
		g->Reflect(aReflector);
	}

	for (auto& g : myGameObjectsToAdd)
	{
		g->Reflect(aReflector);
	}
}

void Engine::GameObjectManager::ReflectAllGameObjectComponents(Reflector & aReflector)
{
	for (auto& g : myGameObjects)
	{
		for (auto& c : g->myComponents)
		{
			c->Reflect(aReflector);
		}
	}

	for (auto& g : myGameObjectsToAdd)
	{
		for (auto& c : g->myComponents)
		{
			c->Reflect(aReflector);
		}
	}
}

void Engine::GameObjectManager::MoveGameObjectToIndex(
	const Shared<GameObject>&aGameObject,
	const int aTargetIndex)
{
	const int index = GetGameObjectIndex(*aGameObject);
	myGameObjects.erase(myGameObjects.begin() + index);

	int newTargetIndex = aTargetIndex;

	// Must correct the target index after erasing something in the vector
	if (aTargetIndex > index)
	{
		newTargetIndex--;
	}

	newTargetIndex = std::max(newTargetIndex, 0);

	myGameObjects.insert(myGameObjects.begin() + newTargetIndex, aGameObject);
}

void Engine::GameObjectManager::UpdatePrefabInstances(
	GameObject * aBeforeChangedPrefabGameObject,
	const GameObjectPrefabRef & aGameObjectPrefab)
{
	// Copy the gameobjects to avoid issues looping issues because we remove gameobjects
	// when updating prefab, and removing within a loop is a bad idea
	// TODO: Loop backwards, solved
	const std::vector<Shared<GameObject>> gameObjects = myGameObjects;

	for (auto& gameObject : gameObjects)
	{
		if (!gameObject->IsPrefabInstance())
		{
			continue;
		}

		GameObject& prefabInstance = *gameObject;
		const auto& prefab = prefabInstance.GetPrefab();

		if (aGameObjectPrefab != prefab)
		{
			continue;
		}

		assert(prefab && prefab->IsValid());

		// When reflecting on each Component & GameObject we assume
		// they their Reflect() layout is exactly the same 1:1

		GameObject* prefabGameObject = aGameObjectPrefab->Get().GetGameObject().get();

		gameObject->SyncWithPrefabFull(aBeforeChangedPrefabGameObject, prefabGameObject);
	}
}

const std::vector<Shared<GameObject>>& Engine::GameObjectManager::GetGameObjects() const
{
	return myGameObjects;
}

int Engine::GameObjectManager::GetGameObjectIndex(const GameObject & aGameObject) const
{
	for (size_t i = 0; i < myGameObjects.size(); ++i)
	{
		if (myGameObjects[i].get() == &aGameObject)
		{
			return static_cast<int>(i);
		}
	}

	return -1;
}

Shared<GameObject> Engine::GameObjectManager::GetGameObjectFromIndex(const int aIndex)
{
	if (aIndex >= 0 && aIndex < myGameObjects.size())
	{
		return myGameObjects[aIndex];
	}

	return nullptr;
}

GameObject* Engine::GameObjectManager::FindGameObject(int aUUID)
{
	for (int i = 0; i < myGameObjectsToAdd.size(); i++)
	{
		if (myGameObjectsToAdd[i]->GetUUID() == aUUID)
			return myGameObjectsToAdd[i].get();
	}

	for (int i = 0; i < myGameObjects.size(); i++)
	{
		if (myGameObjects[i]->GetUUID().GetRef() == aUUID)
			return myGameObjects[i].get();
	}

	return nullptr;
}

const Shared<GameObject>& Engine::GameObjectManager::FindGameObjectShared(const int aUUID)
{
	for (int i = 0; i < myGameObjectsToAdd.size(); i++)
	{
		if (myGameObjectsToAdd[i]->GetUUID() == aUUID)
			return myGameObjectsToAdd[i];
	}

	for (int i = 0; i < myGameObjects.size(); i++)
	{
		if (myGameObjects[i]->GetUUID().GetRef() == aUUID)
			return myGameObjects[i];
	}

	static Shared<GameObject> empty = nullptr;
	return empty;
}

GameObject* Engine::GameObjectManager::FindGameObject(const std::string & aName) const
{
	for (int i = 0; i < myGameObjectsToAdd.size(); i++)
	{
		if (myGameObjectsToAdd[i]->GetName() == aName)
			return myGameObjectsToAdd[i].get();
	}

	for (int i = 0; i < myGameObjects.size(); i++)
	{
		if (myGameObjects[i]->GetName() == aName)
			return myGameObjects[i].get();
	}
	return nullptr;
}

void Engine::GameObjectManager::RemoveDestroyedGameObjects()
{
	std::vector<Shared<GameObject>> gameObjectsToDestroy;

	for (int i = 0; i < myGameObjects.size(); i++)
	{
		auto& g = myGameObjects[i];

		if (g->IsDestroyed())
		{
			gameObjectsToDestroy.push_back(g);
		}
	}

	for (int i = 0; i < gameObjectsToDestroy.size(); i++)
	{
		for (int j = 0; j < myGameObjects.size(); j++)
		{
			if (myGameObjects[j] == gameObjectsToDestroy[i])
			{
				myGameObjects[j] = nullptr;
				myGameObjects.erase(myGameObjects.begin() + j);
			}
		}
	}
}
