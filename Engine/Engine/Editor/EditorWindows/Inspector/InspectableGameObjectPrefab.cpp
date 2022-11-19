#include "pch.h"
#include "InspectableGameObjectPrefab.h"
#include "Engine/ResourceManagement/Resources/GameObjectPrefabResource.h"
#include "Engine/Renderer/Material/Material.h"
#include "Engine/Editor/ImGuiHelper.h"
#include "Engine/Editor/Editor.h"
#include "Engine/Renderer/Material/DecalMaterial.h"
#include "Engine/Editor/DragDropConstants.h"
#include "Engine/Editor/FileTypes.h"
#include "Engine/ResourceManagement/Resources/MaterialResource.h"
#include "Engine/ResourceManagement/Resources/TextureResource.h"
#include "Engine/Editor/EditorWindows/AssetBrowser/AssetBrowserWindow.h"
#include "Engine/Editor/EditorWindows/Inspector/InspectorWindow.h"
#include "Engine/Renderer/Texture/Texture2D.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"
#include "Engine/Scene/FolderScene.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Prefab/GameObjectPrefabInstance.h"
#include "Engine/Reflection/GameObjectPrefabReflector.h"
#include "Engine/GameObject/RecursiveGameObjectChildIterator.h"

Engine::InspectableGameObjectPrefab::InspectableGameObjectPrefab(
	Editor& aEditor,
	InspectorWindow& aInspectorWindow,
	const GameObjectPrefabRef& aGameObjectPrefabResource)
	: Inspectable(aEditor, aInspectorWindow),
	myGameObjectPrefab(aGameObjectPrefabResource),
	myInspectGameObject(aEditor, true, myGameObjectPrefab)
{
	// Must clone it here to save the unchanged values
	// use below to know what values to sync,
	// meaning only sync the values that were changed

	myUnchangedPrefabGameObject = myGameObjectPrefab->Get().GetGameObject()->Clone(myPlaceholderGameObjectManager, false);

	myPrefabGameObjectCurrentlyInspected = myGameObjectPrefab->Get().GetGameObject();
}

void Engine::InspectableGameObjectPrefab::Draw()
{
	Inspectable::Draw();

	// if (!myGameObjectPrefab || !myGameObjectPrefab->IsValid())
	// {
	// 	ImGui::Text("No prefab selected, or invalid, select one in the asset browser");
	// 	return;
	// }

	auto& prefab = myGameObjectPrefab->Get();

	// There is no direct connection to the prefab instances
	// Simply, when i change a value in this inspector
	// loop through all gameobjects in the scene and update them
	// but how will it work in other scenes?
	// when opening another scene
	// it must first off load the prefab resources.
	// then the PrefabInstance must be saved with its prefab resource.
	// and when loaded it must somehow, create the gameobject from the Prefab using InstantiatePrefabInstance()
	// and then load its own changes to override them NVM, NOT NEEDED THEY CONTAIN ALL THE STATE THEY
	// NEED WHEN SAVED. SO WE ONLY NEED TO REFLECT ON THE DIFFERENCES since it contains a 
	// reference to its GameObjectPrefab.

	// When syncing the "Add Component", override AddComponent() in GameObjectPrefabInstance

	Scene& scene = *myEditor.GetActiveScene();
	myInspectGameObject.DrawInspectedGameObject(scene, myPrefabGameObjectCurrentlyInspected);

	//if (ImGui::Button("Save & Update Instances"))
	//{
	//	myInspectorWindow.Save();
	//	// Save();
	//}

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20);
	if (ImGui::Button("Save & Update Instances", ImVec2(-20, 0)))
	{
		myInspectorWindow.Save();
	}
}

void Engine::InspectableGameObjectPrefab::Save()
{
	if (!myGameObjectPrefab || !myGameObjectPrefab->IsValid())
	{
		return;
	}

	if (myGameObjectPrefab->Get().Serialize(myGameObjectPrefab->GetPath()))
	{
		UpdateExistingPrefabs();
	}
}

void Engine::InspectableGameObjectPrefab::SetPrefabGameObject(const Shared<GameObject>& aGameObject)
{
	myPrefabGameObjectCurrentlyInspected = aGameObject;
}

void Engine::InspectableGameObjectPrefab::UpdateExistingPrefabs()
{
	// If no active scene when saving the prefab
	if (myEditor.GetActiveScene() == nullptr)
	{
		return;
	}

	// Copy the gameobject vector because interally in SyncWithPrefabFull()
	// we modify this vector directly when removing gameobjects
	// TODO: Might become poor performance in future, then look at
	// solution to have a middle man vector that is called myGameObjectsToRemove

	myEditor.GetActiveScene()->myGameObjectManager.UpdatePrefabInstances(
		myUnchangedPrefabGameObject.get(), 
		myGameObjectPrefab);

	//const std::vector<Shared<GameObject>> gameObjects = myEditor.GetActiveScene()->myGameObjectManager.myGameObjects;

	//for (auto& gameObject : gameObjects)
	//{
	//	if (!gameObject->IsPrefabInstance())
	//	{
	//		continue;
	//	}

	//	GameObject& prefabInstance = *gameObject;
	//	const auto& prefab = prefabInstance.GetPrefab();

	//	if (myGameObjectPrefab != prefab)
	//	{
	//		continue;
	//	}

	//	assert(prefab && prefab->IsValid());

	//	// When reflecting on each Component & GameObject we assume
	//	// they their Reflect() layout is exactly the same 1:1

	//	GameObject* prefabGameObject = myGameObjectPrefab->Get().myGameObject.get();

	//	gameObject->SyncWithPrefabFull(myUnchangedPrefabGameObject.get(), prefabGameObject);
	//}

	// Update our unchanged prefab gameobject to compare with the proper changes
	// when pressing Save again
	myUnchangedPrefabGameObject = myGameObjectPrefab->Get().GetGameObject()->Clone(myPlaceholderGameObjectManager, false);
}
