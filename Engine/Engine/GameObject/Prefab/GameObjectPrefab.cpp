#include "pch.h"
#include "GameObjectPrefab.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine\Reflection\JsonDeserializerReflector.h"
#include "Engine\Scene\Scene.h"
#include "Engine\Reflection\JsonSerializerReflector.h"
#include "Engine/Reflection/GameObjectPrefabReferencesReflector.h"
#include "Engine\Editor\FileTypes.h"
#include <Engine/Reflection/ResourceRefReflector.h>
#include "Engine/ResourceReferences.h"
#include "GameObjectPrefabInstance.h"
#include "Engine/ResourceManagement/Resources/GameObjectPrefabResource.h"
#include "Engine/GameObject/RecursiveGameObjectChildIterator.h"

bool Engine::GameObjectPrefab::Serialize(GameObject& aGameObject, const Path& aPath)
{
	// Go through all children and give them a unique prefabGameObjectID
	int prefabGameObjectIDCounter = 0;

	aGameObject.myPrefabGameObjectID = prefabGameObjectIDCounter++;

	for (auto it = GameObjectChildIterator(aGameObject); it != nullptr; it++)
	{
		it.DerefAsRaw()->myPrefabGameObjectID = prefabGameObjectIDCounter++;
	}

	nlohmann::json gameObjectJson;

	SerializeGameObject(gameObjectJson, aGameObject);

	nlohmann::json j;

	j["GameObject"] = gameObjectJson.at(0);

	if (!IsPrefabIDCounterValid(prefabGameObjectIDCounter, aGameObject))
	{
		LOG_ERROR(LogType::Resource) << "Prefab ID counter is not valid, skipping. " << aPath;
		return false;
	}

	j["PrefabGameObjectIDCounter"] = prefabGameObjectIDCounter;

	FileIO::RemoveReadOnly(aPath.AppendPath(aGameObject.GetName() + FileType::Extension_GameObjectPrefab));

	std::ofstream file(aPath.AppendPath(aGameObject.GetName() + FileType::Extension_GameObjectPrefab).ToWString());

	if (file.is_open())
	{
		file << std::setw(4) << j;
		file.close();
	}
	else
	{
		LOG_ERROR(LogType::Engine) << "Failed to save prefab, is the folder read only?";
	}

	return true;
}

bool Engine::GameObjectPrefab::Serialize(const Path& aPath)
{
	FileIO::RemoveReadOnly(aPath);

	std::ofstream file(aPath.ToWString());

	nlohmann::json j;
	j["GameObject"] = ToJson();

	if (!IsPrefabIDCounterValid(myPrefabGameObjectIDCounter, *myGameObject))
	{
		LOG_ERROR(LogType::Resource) << "Prefab ID counter is not valid, skipping. " << aPath;
		return false;
	}

	j["PrefabGameObjectIDCounter"] = myPrefabGameObjectIDCounter;

	if (file.is_open())
	{
		file << std::setw(4) << j;
		file.close();
	}
	else
	{
		LOG_ERROR(LogType::Engine) << "Failed to save material";
		return false;
	}

	return true;
}

bool Engine::GameObjectPrefab::Deserialize(const Path& aPath, const bool aLoadResourceNow)
{
	std::ifstream file(aPath.ToString());

	if (!file.is_open())
	{
		LOG_ERROR(LogType::Resource) << "Failed to open " << aPath;
		return false;
	}

	const bool ignoreComments = true;
	const bool allowExceptions = false;
	auto j = nlohmann::json::parse(file, nullptr, allowExceptions, ignoreComments);

	// If failed to parse json file
	if (j.is_discarded())
	{
		LOG_ERROR(LogType::Resource) << "Failed to parse " << aPath;
		return false;
	}

	if (j.contains("GameObject"))
	{
		myGameObject = DeserializeGameObject(j["GameObject"], nullptr);

		// All gameobjects in a prefab must have unique prefab
		// gameobject ID's
		if (!AreAllPrefabGameObjectIDUnique())
		{
			LOG_ERROR(LogType::Resource) << "Prefab does not have unique IDs, skipping. " << aPath;
			return false;
		}
	}
	else
	{
		LOG_ERROR(LogType::Resource) << "Prefab is corrupt" << aPath;
		return false;
	}

	if (j.contains("PrefabGameObjectIDCounter"))
	{
		myPrefabGameObjectIDCounter = j["PrefabGameObjectIDCounter"];

		if (!IsPrefabIDCounterValid(myPrefabGameObjectIDCounter, *myGameObject))
		{
			LOG_WARNING(LogType::Resource) << "Prefab had invalid ID counter, recalculating it: " << aPath;
			GenerateNewIDCounter();
		}
	}
	else
	{
		LOG_WARNING(LogType::Resource) << "Prefab missing ID counter, generating a new one: " << aPath;

		GenerateNewIDCounter();

		/*
		myPrefabGameObjectIDCounter = 0;
		LOG_ERROR(LogType::Resource) << "Prefab is corrupt: missing \"PrefabGameObjectIDCounter\"" << aPath;
		return false;
		*/
	}

	return true;
}

bool Engine::GameObjectPrefab::Deserialize2(const nlohmann::json& aGameObjectJson)
{
	myGameObject = DeserializeGameObject(aGameObjectJson, nullptr);

	// All gameobjects in a prefab must have unique prefab
	// gameobject ID's
	if (!AreAllPrefabGameObjectIDUnique())
	{
		LOG_ERROR(LogType::Resource) << "Prefab does not have unique IDs, skipping. ";
		return false;
	}

	return true;
}

nlohmann::json Engine::GameObjectPrefab::ToJson() const
{
	nlohmann::json gameObjectJson;
	SerializeGameObject(gameObjectJson, *myGameObject);
	return gameObjectJson.at(0);
}

GameObject& Engine::GameObjectPrefab::Instantiate(Scene& aScene)
{
	// TODO: Update the references to GameObject and Component here
	// using a new reflector

	// auto g = aScene.AddGameObjectForEditor2();

	auto go = myGameObject->Clone(aScene.myGameObjectManager, true);

	// Update the prefab gameobject references
	GameObjectPrefabReferencesReflector refl(aScene.myGameObjectManager, *go);
	go->ReflectChildrenAndComponents(refl);

	// GameObjectPrefabReferencesReflector refl(aScene.myGameObjectManager, *myGameObject, *go);
	// go->ReflectChildrenAndComponents(refl);

	//Shared<GameObject> go = MakeShared<GameObject>(*myGameObject);
	//
	//go->UpdateInternalReferences(&aScene);

	// Update the references of GameObject, Component
	// GameObjectPrefabReferencesReflector reflector(*go);
	// go->Reflect(reflector);

	// If a component has exposed a reference to a GameObject
	// or another Component, we update their references to
	// this new GameObject's values
	// So, if the GameObject you made the prefab of, one of its Component's had
	// a reference to another Component, we update that reference so its valid
	// for this newly allocated GameObject
	// for (Component* c : go->myComponents)
	// {
	// 	GameObjectPrefabReferencesReflector reflector2(*go);
	// 	c->Reflect(reflector2);
	// }

	// aScene.AddGameObjectForEditor(go);
	// aScene.myGameObjectManager.AddGameObject2(go);

	return *go;
}

GameObject& Engine::GameObjectPrefab::InstantiatePrefabInstance(
	const GameObjectPrefabRef& aPrefab, Scene& aScene)
{
	auto go = aPrefab->Get().myGameObject->CloneAsPrefabInstance(aScene.myGameObjectManager, true);

	// auto go = myGameObject->CloneAsPrefabInstance(aScene.myGameObjectManager, true);

	// Only top level prefab gameobject should have the prefab resource
	// because when looping through all prefabs in the scene to save the changes
	// made to the prefab, we then loop through the children of that top level prefab
	// to save those changes as well
	go->myPrefab = aPrefab;
	go->myPrefabGameObject = aPrefab->Get().GetGameObject().get();

	// hur fan ska jag länka children till prefabben?

	// We assume the gameobject hierarchy is 1:!
	// iterate the hierarchy and match each one to the respective gameobject as a prefab

	auto it = GameObjectChildIterator(*go);
	auto it2 = GameObjectChildIterator(*aPrefab->Get().GetGameObject());

	for (; it != nullptr && it2 != nullptr; it++, it2++)
	{
		// it.DerefAsRaw()->myPrefab = aPrefab;
		it.DerefAsRaw()->myPrefabGameObject = it2.DerefAsRaw();
	}

	return *go;
}

const Shared<GameObject>& Engine::GameObjectPrefab::GetGameObject() const
{
	return myGameObject;
}

Shared<GameObject> Engine::GameObjectPrefab::CreateGameObjectChild()
{
	auto g = MakeShared<GameObject>();

	// keep child alive
	myAllGameObjects.push_back(g);

	g->GetTransform().SetParent2(&myGameObject->GetTransform());
	g->GetTransform().myGameObject = g;

	// Generate a new ID that is unique to this gameboejct prefab hierarchy
	{
		g->myPrefabGameObjectID = GetNextPrefabGameObjectID();
	}

	return g;
}

void Engine::GameObjectPrefab::RemoveGameObjectChild(const Shared<GameObject>& aGameObject)
{
	for (auto it = GameObjectChildIterator(*myGameObject); it != nullptr; it++)
	{
		if (it.DerefAsRaw() == aGameObject.get())
		{
			auto findResult = std::find_if(
				myAllGameObjects.begin(),
				myAllGameObjects.end(),
				[&it](const Shared<GameObject>& aGameObject)
				{
					return aGameObject.get() == it.DerefAsRaw();
				});

			if (findResult != myAllGameObjects.end())
			{
				myAllGameObjects.erase(findResult);
			}

			it.DerefAsRaw()->GetTransform().GetParent()->RemoveChild(&it.DerefAsRaw()->GetTransform());

			return;
		}
	}
}

int Engine::GameObjectPrefab::GetNextPrefabGameObjectID()
{
	return myPrefabGameObjectIDCounter++;
}

void Engine::GameObjectPrefab::SerializeGameObject(
	nlohmann::json& aArrayOut, GameObject& aGameObject)
{
	JsonSerializerReflector jsonReflector;

	aGameObject.Reflect(jsonReflector);

	nlohmann::json componentsArrayJson = nlohmann::json::array();

	for (Component* c : aGameObject.myComponents)
	{
		JsonSerializerReflector jsonReflector2;
		c->Reflect(jsonReflector2);
		auto componentJson = jsonReflector2.GetJson();

		// We dont save unregistered components, should not even be possible
		// unless we save a runtime scene
		if (auto regComponent = ComponentFactory::GetInstance().GetRegisteredComponentFromId(c->GetComponentTypeID()))
		{
			componentJson["Type"] = regComponent->myName;
			componentsArrayJson.push_back(componentJson);
		}
	}

	auto gameObjectJson = jsonReflector.GetJson();

	gameObjectJson["Components"] = componentsArrayJson;

	auto childrenArrayJson = nlohmann::json::array();

	for (const auto& childTransform : aGameObject.GetTransform().GetChildren())
	{
		SerializeGameObject(childrenArrayJson, *childTransform->GetGameObject());
	}

	gameObjectJson["Children"] = childrenArrayJson;

	aArrayOut.push_back(gameObjectJson);
}

Shared<GameObject> Engine::GameObjectPrefab::DeserializeGameObject(
	const nlohmann::json& aGameObjectJson,
	const Shared<GameObject>& aParent)
{
	// Me and Jesper:
	// ugly fix to the issue where a race condition crash occured when loaded a resource that was already requested
	// Therefore, we request it here as well, and not load immediately
	const bool requestNow = true;
	JsonDeserializerReflector reflector(&aGameObjectJson, requestNow);

	auto g = MakeShared<GameObject>();

	// keep children alive
	myAllGameObjects.push_back(g);

	// NOTE(filip):
	// Must call SetParent() before doing the Reflect and setting the Transform values
	// Otherwise SetParent() ruins the position, dont know why, dont have time
	// to find out atm
	if (aParent)
	{
		g->GetTransform().SetParent2(&aParent->GetTransform());
	}

	g->GetTransform().myGameObject = g;

	g->Reflect(reflector);

	for (const auto& componentJson : aGameObjectJson["Components"])
	{
		const std::string type = componentJson["Type"];

		JsonDeserializerReflector reflector2(&componentJson, requestNow);

		std::hash<std::string> h;
		const auto registeredComponent
			= ComponentFactory::GetInstance().GetRegisteredComponentFromComponentNameHash(h(type));

		// how can we load an unregistered component, it was not supposed to be saved
		if (!registeredComponent)
		{
			LOG_ERROR(LogType::Engine) << "Prefab gameobject has null component " << type;
			continue;
		}

		// add manually into gameobject manager lol yieks haha
		if (auto component = registeredComponent->myComponent->Clone())
		{
			//g->AddExistingComponent(component);
			g->myComponents.push_back(component);

			component->Reflect(reflector2);
		}
		else
		{
			// If it came here means that Clone() was not implemented by the component
			// however, its impossible since it was able to create and save it in the first
			// place, so something bad has occured
			assert(false);
		}
	}

	for (const auto& childGameObjectJson : aGameObjectJson["Children"])
	{
		DeserializeGameObject(childGameObjectJson, g);
	}

	return g;
}

bool Engine::GameObjectPrefab::AreAllPrefabGameObjectIDUnique() const
{
	std::set<int> set;

	for (const auto& g : myAllGameObjects)
	{
		if (set.find(g->GetPrefabGameObjectID()) == set.end())
		{
			set.insert(g->GetPrefabGameObjectID());
		}
		else
		{
			return false;
		}
	}

	return true;
}

bool Engine::GameObjectPrefab::IsPrefabIDCounterValid(const int aIDCounterValue, GameObject& aTopPrefabGameObject)
{
	for (auto it = GameObjectChildIterator(aTopPrefabGameObject); it != nullptr; it++)
	{
		// The ID counter value must be bigger than any gameobject prefab ID
		if (it.DerefAsRaw()->GetPrefabGameObjectID() > aIDCounterValue)
		{
			return false;
		}
	}

	return true;
}

void Engine::GameObjectPrefab::GenerateNewIDCounter()
{
	int newCounterValue = 0;
	for (auto it = GameObjectChildIterator(*myGameObject); it != nullptr; it++)
	{
		// The ID counter value must be bigger than any gameobject prefab ID
		newCounterValue = std::max(it.DerefAsRaw()->GetPrefabGameObjectID(), newCounterValue);
	}

	myPrefabGameObjectIDCounter = newCounterValue + 1;
}
