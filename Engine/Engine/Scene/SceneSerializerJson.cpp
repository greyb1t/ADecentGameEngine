#include "pch.h"
#include "SceneSerializerJson.h"

#include "FolderScene.h"
#include "Engine/Reflection/JsonSerializerReflector.h"
#include "Engine/Reflection/JsonDeserializerReflector.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine\GameObject\Prefab\GameObjectPrefab.h"
#include "Engine\ResourceManagement\Resources\GameObjectPrefabResource.h"

Engine::SceneSerializerJson::~SceneSerializerJson()
{
}

void Engine::SceneSerializerJson::Serialize(FolderScene& aFolderScene, const Path& aPath)
{
	nlohmann::json gameObjectArrayJson = nlohmann::json::array();

	SerializeGameObjects(gameObjectArrayJson, aFolderScene.myGameObjectManager.myGameObjects);
	SerializeGameObjects(gameObjectArrayJson, aFolderScene.myGameObjectManager.myGameObjectsToAdd);

	nlohmann::json j;

	j["Name"] = aFolderScene.GetName();
	j["GameObjects"] = gameObjectArrayJson;

	std::unordered_map<std::string, Shared<GameObject>> gameObjectPrefabsUsedInTheScene;

	// iterate the gameobject and add the prefabs used here

	std::vector<Shared<GameObject>> allGameObjects;
	allGameObjects.insert(
		allGameObjects.begin(),
		aFolderScene.myGameObjectManager.myGameObjects.begin(),
		aFolderScene.myGameObjectManager.myGameObjects.end());
	allGameObjects.insert(
		allGameObjects.begin(),
		aFolderScene.myGameObjectManager.myGameObjectsToAdd.begin(),
		aFolderScene.myGameObjectManager.myGameObjectsToAdd.end());

	for (const auto& g : allGameObjects)
	{
		if (g->GetPrefab() && g->GetPrefab()->IsValid())
		{
			gameObjectPrefabsUsedInTheScene[g->GetPrefab()->GetPath()] = g->GetPrefab()->Get().GetGameObject();

			// temporarily to make it save the prefab whencalling SerializeGameObjects() below
			// this must be reset because otherwise its a circular reference with shared_ptr
			gameObjectPrefabsUsedInTheScene[g->GetPrefab()->GetPath()]->myPrefab = g->GetPrefab();
		}
	}

	// Serialize the gameobject prefabs in their current state
	// this is need to later on when loading the scene to compare and update prefabs
	// that has been changed from another scene
	std::vector<Shared<GameObject>> goos;
	for (auto& [prefabPath, prefabGameObject] : gameObjectPrefabsUsedInTheScene)
	{
		goos.push_back(prefabGameObject);
	}

	auto prefabGameObjectsJson = nlohmann::json::array();
	SerializeGameObjects(prefabGameObjectsJson, goos);
	j["SavedPrefabGameObjectsStates"] = prefabGameObjectsJson;

	// Must remove the circular reference mentioned above
	// otherwise memory leak, cringe
	for (auto& [prefabPath, prefabGameObject] : gameObjectPrefabsUsedInTheScene)
	{
		prefabGameObject->myPrefab = { };
	}

	FileIO::RemoveReadOnly(aPath);

	std::ofstream file(aPath.ToWString());

	if (file.is_open())
	{
		file << std::setw(4) << j;
		file.close();
	}
	else
	{
		LOG_ERROR(LogType::Engine) << "Failed to save scene, is it read only?";
	}
}

bool Engine::SceneSerializerJson::Deserialize(FolderScene& aFolderScene, const Path& aPath)
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

	// aFolderScene.myName = j["Name"];

	//#ifdef GAMEOBJECT_PREFAB
	for (const auto& savedPrefabGameObjectJson : j["SavedPrefabGameObjectsStates"])
	{
		// const std::string prefabGameObjectPath = savedPrefabGameObjectJson["Path"];

		// DeserializeGameObject(savedPrefabGameObjectJson, aFolderScene, nullptr);

		auto gameObjectPrefab = MakeShared<GameObjectPrefab>();
		gameObjectPrefab->Deserialize2(savedPrefabGameObjectJson);
		mySavedPrefabGameObjects[gameObjectPrefab->GetGameObject()->GetPrefab()->GetPath()] = gameObjectPrefab;
	}
	//#endif

	for (const auto& gameObjectJson : j["GameObjects"])
	{
		DeserializeGameObject(gameObjectJson, aFolderScene, nullptr);
	}

	return true;
}

void Engine::SceneSerializerJson::SerializeGameObjects(
	nlohmann::json& aArrayOut,
	const std::vector<Shared<GameObject>>& aGameObjects)
{
	for (auto& gameobject : aGameObjects)
	{
		if (gameobject->IsFromUnreal())
		{
			continue;
		}

		// Only do this for the top level gameobjects
		// the children will be done with recursion
		if (gameobject->myTransform.GetParent() != nullptr)
		{
			continue;
		}

		SerializeGameObject(aArrayOut, *gameobject);
	}
}

void Engine::SceneSerializerJson::SerializeGameObject(
	nlohmann::json& aArrayOut,
	GameObject& aGameObject)
{
	// nlohmann::json gameObjectJson;

	JsonSerializerReflector jsonReflector;

	aGameObject.Reflect(jsonReflector);

	nlohmann::json componentsArrayJson = nlohmann::json::array();

	for (Component* c : aGameObject.myComponents)
	{
		JsonSerializerReflector jsonReflector2;
		c->Reflect(jsonReflector2);
		auto componentJson = jsonReflector2.GetJson();
		auto regComponent = ComponentFactory::GetInstance().GetRegisteredComponentFromId(c->GetComponentTypeID());

		if (regComponent)
		{
			componentJson["Type"] = regComponent->myName;
			componentsArrayJson.push_back(componentJson);
		}
		else
		{
			LOG_ERROR(LogType::Engine) << "Tried to save unregistered component on GameObject: " << aGameObject.GetName();
		}
	}

	auto gameObjectJson = jsonReflector.GetJson();

	gameObjectJson["Components"] = componentsArrayJson;

	auto childrenArrayJson = nlohmann::json::array();

	for (const auto& childTransform : aGameObject.GetTransform().GetChildren())
	{
		// childTransform->GetGameObject()
		SerializeGameObject(childrenArrayJson, *childTransform->GetGameObject());
	}

	gameObjectJson["Children"] = childrenArrayJson;

	//gameObjectJson["GameObject"] = jsonReflector.GetJson();
	//gameObjectJson["Components"] = componentsArrayJson;
	//
	//gameObjectJson["Children"] = ;

	aArrayOut.push_back(gameObjectJson);
}

void Engine::SceneSerializerJson::DeserializeGameObject(
	const nlohmann::json& aGameObjectJson,
	FolderScene& aFolderScene,
	GameObject* aParent)
{
	JsonDeserializerReflector reflector(&aGameObjectJson);

	auto g = aFolderScene.AddGameObject<GameObject>();

	// NOTE(filip):
	// Must call SetParent() before doing the Reflect and setting the Transform values
	// Otherwise SetParent() ruins the position, dont know why, dont have time
	// to find out atm
	if (aParent)
	{
		g->GetTransform().SetParent(&aParent->GetTransform());
	}

	// g->Reflect(reflector);


	for (const auto& componentJson : aGameObjectJson["Components"])
	{
		const std::string type = componentJson["Type"];

		JsonDeserializerReflector reflector2(&componentJson);

		std::hash<std::string> h;
		const auto registeredComponent =
			ComponentFactory::GetInstance().GetRegisteredComponentFromComponentNameHash(h(type));

		if (registeredComponent)
		{
			// add manually into gameobject manager lol yieks haha
			if (auto component = registeredComponent->myComponent->Clone())
			{
				g->AddExistingComponent(component);

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
		else
		{
			LOG_ERROR(LogType::Engine) << "Tried to loading unregistered component: " << type;
		}
	}

	for (const auto& childGameObjectJson : aGameObjectJson["Children"])
	{
		DeserializeGameObject(childGameObjectJson, aFolderScene, g);
	}

	// We reflect AFTER the whole gameobject hierarchy was created
	// need for the prefab to update the gameobject according to the prefab 1:1
	g->Reflect(reflector);

	// Must reflect separate here because we cannot do it in GameObject
	// why? because we need direct access to saved gameobject prefabs in the scene
	// because we need those to compare against when loading a scene with the prefab
	// having been modified in another scene
	// When saving a scene we also save the used prefab's gameobject's to compare against
	// when loading the scene again, because prefab might have been modified in another scene
	// then when loading this scene, we need to update those changes

	g->ReflectPrefab(reflector, mySavedPrefabGameObjects);
}
