#include "pch.h"
#include "Editor.h"

#include "imgui_internal.h"
#include "Command/EditorCommandComposite.h"
#include "Command/Commands/AddComponentCommand.h"
#include "Command/Commands/AddGameObjectCommand.h"
#include "Command/Commands/RemoveComponentCommand.h"
#include "Command/Commands/SetSelectionGameObjectsCommand.h"
#include "Command/Commands/RemoveGameObjectCommand.h"
#include "Common/FileIO.h"
#include "EditorWindows/AnimationCurveEditorWindow.h"
#include "EditorWindows/Inspector/InspectorWindow.h"
#include "EditorWindows/LogWindow.h"
#include "EditorWindows/SceneHierarchy/SceneHierarchyWindow.h"
#include "Engine/Editor/EditorWindows/AssetBrowser/AssetBrowserWindow.h"
#include "EditorWindows/AnimationNodeEditorWindow.h"
#include "Engine/GameObject/Components/BasicMovementComponent.h"
#include "EditorWindows\LogWindow.h"
#include "EditorWindows\VisualScriptWindow.h"
#include "EditorWindows\SceneWindow.h"
#include "Engine/Time.h"
#include "Engine/Animation/AnimationStateMachine.h"
#include "Engine/Animation/State/AnimationMachine.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/EditorCameraMovement.h"
#include "Engine/GameObject/Components/CameraComponent.h"
#include "Engine/GameObject/Components/CameraShakeComponent.h"
#include "Engine/Reflection/JsonSerializerReflector.h"
#include "Engine/Renderer/Animation/AssimpUtils.h"
#include "Engine/ResourceManagement/ResourceManager.h"
#include "Engine/ResourceManagement/Resources/TextureResource.h"
#include "Engine/Scene/SceneManager.h"
#include "Engine/Scene/SceneRegistry.h"
#include "Engine/Scene/SceneSerializerJson.h"
#include "Engine/Scene\FolderScene.h"
#include "Engine/GameObject/Components/EnvironmentLightComponent.h"
#include "Engine/GraphManager/VisualScript.h"
#include "Engine/Reflection/ResourceRefReflector.h"
#include "Engine/Renderer/Camera/CameraFactory.h"
#include "Engine/ResourceManagement/Resources/AnimationStateMachineResource.h"
#include "Engine/ResourceManagement/Resources/VisualScriptResource.h"
#include "EditorWindows/AssetBrowser/Importing/ImportWindow.h"
#include "EditorWindows/MaterialWindow.h"
#include "Engine/ResourceManagement/Resources/MaterialResource.h"
#include "Engine/ResourceManagement/Resources/GameObjectPrefabResource.h"
#include "Command/Commands/MoveGameObjectCommand.h"
#include "EditorWindows/RayCastAtPlaneWindow.h"
#include "EditorWindows/Inspector/InspectableSelection.h"
#include "EditorWindows/Inspector/InspectableMaterial.h"
#include "EditorWindows/Inspector/InspectableGameObjectPrefab.h"
#include "Command/Commands/TransformCommand.h"
#include "Engine/GameObject/Components/PostProcessComponent.h"
#include "Engine/TimeSystem/TimeSystem.h"
#include <Engine/AudioManager.h>
#include "EditorWindows/MakeRetailBuildWindow.h"
#include "Engine/Paths.h"
#include "Engine/AssetDatabase.h"

Engine::Editor::Editor()
	: myEditingScene(*this),
	myRuntimeScene(*this)
{
	myName = "EditorScene";
}

Engine::Editor::~Editor()
{
	SaveToFile();
}

bool Engine::Editor::Init()
{
	if (!Scene::Init())
	{
		assert(false);
		return false;
	}

	AssetDatabase::ToggleAutoUpdateAssetCache(true);

	// Just so the engine does not cry because the editor scene does
	// not have a camera or environment light, they are not even used
	CreateDefaultCamera();
	CreateDefaultEnvironmentLight();

	mySceneHierarchy = MakeOwned<SceneHierarchyWindow>(*this);
	myInspectorWindow = MakeOwned<InspectorWindow>(*this);
	myInspectorWindow->Init(myResourceReferences);

	myAssetBrowsers[0] = MakeOwned<AssetBrowserWindow>(*this, 0);
	myAssetBrowsers[0]->Init(myResourceReferences);

	myLogWindow = MakeOwned<LogWindow>(*this);

	mySceneWindow = MakeOwned<SceneWindow>(*this, myResourceReferences);

	LoadSaveData();

	myResourceReferences.AddTexture("Assets/Engine/Editor/Bjorn.dds");
	myResourceReferences.AddTexture("Assets/Engine/Editor/BlamePontus.dds");
	myResourceReferences.AddTexture("Assets/Engine/Editor/buddies.dds");
	myResourceReferences.AddTexture("Assets/Engine/Editor/CogDog.dds");
	myResourceReferences.AddTexture("Assets/Engine/Editor/Compiles.dds");
	myResourceReferences.AddTexture("Assets/Engine/Editor/David.dds");
	myResourceReferences.AddTexture("Assets/Engine/Editor/DoItForHer.dds");
	myResourceReferences.AddTexture("Assets/Engine/Editor/Filip.dds");
	myResourceReferences.AddTexture("Assets/Engine/Editor/FuckYou.dds");
	myResourceReferences.AddTexture("Assets/Engine/Editor/godis.dds");
	myResourceReferences.AddTexture("Assets/Engine/Editor/grab.dds");
	myResourceReferences.AddTexture("Assets/Engine/Editor/Hello.dds");
	myResourceReferences.AddTexture("Assets/Engine/Editor/Jacky.dds");
	myResourceReferences.AddTexture("Assets/Engine/Editor/kings.dds");
	myResourceReferences.AddTexture("Assets/Engine/Editor/NoFace.dds");
	myResourceReferences.AddTexture("Assets/Engine/Editor/Pathetic.dds");
	myResourceReferences.AddTexture("Assets/Engine/Editor/pog.dds");
	myResourceReferences.AddTexture("Assets/Engine/Editor/MethIsMeth.dds");
	myResourceReferences.AddTexture("Assets/Engine/Editor/CaptainQuaternion.dds");
	myResourceReferences.AddTexture("Assets/Engine/Editor/TheProphecisedLD.dds");
	myResourceReferences.AddTexture("Assets/Engine/Editor/CoggusDoggus.dds");
	myResourceReferences.AddTexture("Assets/Engine/Editor/pog.dds");
	myResourceReferences.AddTexture("Assets/Engine/Editor/Pontus.dds");
	myResourceReferences.AddTexture("Assets/Engine/Editor/Pontus2.dds");
	myResourceReferences.AddTexture("Assets/Engine/Editor/ShrekMike.dds");
	myResourceReferences.AddTexture("Assets/Engine/Editor/Tommy.dds");
	myResourceReferences.AddTexture("Assets/Engine/Editor/William.dds");
	myResourceReferences.AddTexture("Assets/Engine/Editor/WingDings.dds");


	myResourceReferences.RequestAll();

	return true;
}

void Engine::Editor::Update(TimeStamp ts)
{
	ZoneNamedN(zone1, "Editor::Update", true);

	Scene::Update(ts);

	UpdateMoveToCameraLerp();

	if (myEditorCamera)
	{
		myEditorCameraPosition = myEditorCamera->GetTransform().GetPosition();
		myEditorCameraRotation = myEditorCamera->GetTransform().GetRotation();
		myEditorCameraRotationForComponent = myEditorCamMovement->GetCurrentRotation();
	}

	if (myEditingScene.GetScene())
	{
		// When adding gameobjects in the editor, they get added to the
		// gameObjectToAdd, but that never used because we never
		// call Update() on the editing scene to avoid changing anthing in runtime
		// the scene must be static and unchanged, therefore no runtime stuff and updating
		myEditingScene.GetScene()->myGameObjectManager.CreateQueuedGameObjects();
		// ADD EXECUTE
		// NOTE(filip): EditorUpdate is already run through SceneManager.cpp
		// myEditingScene.GetScene()->myGameObjectManager.EditorUpdate();
		myEditingScene.GetScene()->myGameObjectManager.RemoveDestroyedGameObjects();
	}

	if (myRuntimeScene.GetScene())
	{
		ZoneNamedN(zone2, "Editor::Update::ClonePart", true);

		// When we do not have a editing scene, we will create it from the runtime
		// scene by cloning it
		if (myEditingScene.GetScene() == nullptr
			&& myRuntimeScene.GetScene()->GetInitState() == Scene::InitState::RuntimeInit)
		{
			// Clone the scene before actually creating the runtime gameobjects
			const auto clonedScene = dynamic_cast<FolderScene*>(myRuntimeScene.GetScene()->Clone());

			if (clonedScene != nullptr)
			{
				LoadSceneSpecificSaveData(clonedScene->GetFolderPath().GetStem().ToString());

				CloneAndSetScene(std::shared_ptr<FolderScene>(clonedScene));

				LOG_INFO(LogType::Filip) << "Cloned scene: " << clonedScene->GetFolderPath();

				// When we cloned the runtime scene, just kill it
				// we'll create a new when when clicking the Play button
				myRuntimeScene.SetScene(nullptr);
				GetEngine().GetSceneManager().RemoveScene(myRuntimeSceneHandle);
			}
			else
			{
				LOG_WARNING(LogType::Engine)
					<< "Scene: " << myRuntimeScene.GetScene()->GetName() << " is not cloneable.";
			}
		}
	}

	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the 
	// parent window not dockable into, because it would be 
	// confusing to have two docking targets within each others.
	ImGuiWindowFlags window_flags =
		ImGuiWindowFlags_MenuBar |
		ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoNavFocus;

	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

	// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
	// and handle the pass-thru hole, so we ask Begin() to not render a background.
	// This will render the game in the background instead :)
	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
	{
		window_flags |= ImGuiWindowFlags_NoBackground;
	}

	// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
	// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
	// all active windows docked into it will lose their parent and become undocked.
	// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
	// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	static bool p_open = true;
	ImGui::Begin("Editor", &p_open, window_flags);

	ImGui::PopStyleVar();

	ImGui::PopStyleVar(2);

	ImGuiID dockspace_id = ImGui::GetID(ourDockSpaceId);
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

	if (myResetDockingLayout)
	{
		myResetDockingLayout = false;

		ResetDockingLayout();
	}

	if (ImGui::BeginMenuBar())
	{
		ZoneNamedN(zone3, "Editor::Update::MenuBar", true);

		DrawFileMenu();

		DrawEditMenu();

		DrawViewMenu();

		DrawToolsMenu();

		if (ImGui::BeginMenu("Windows"))
		{
			if (ImGui::MenuItem("Style editor"))
			{
				myShowStyleEditor = !myShowStyleEditor;
			}

			if (ImGui::MenuItem("Animation Clip Importer"))
			{
				OpenImportWindow(ResourceImportState::AnimationClip);
			}

			if (ImGui::MenuItem("Model Importer"))
			{
				OpenImportWindow(ResourceImportState::Model);
			}

			if (ImGui::MenuItem("Reset Layout"))
			{
				myResetDockingLayout = true;
			}

			if (ImGui::BeginMenu("Asset Browser"))
			{
				// Start at 1 because we do not allow turning off the first asset
				// browser because other stuff are dependant on it
				for (size_t i = 1; i < myAssetBrowsers.size(); ++i)
				{
					if (ImGui::MenuItem(
						("Asset Browser " + std::to_string(i)).c_str(),
						nullptr,
						myAssetBrowsers[i] != nullptr))
					{
						if (myAssetBrowsers[i] != nullptr)
						{
							myAssetBrowsers[i] = nullptr;
						}
						else
						{
							myAssetBrowsers[i] = MakeOwned<AssetBrowserWindow>(*this, i);
							myAssetBrowsers[i]->Init(myResourceReferences);
							myResourceReferences.RequestAll();
						}
					}
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Tools"))
		{
			if (ImGui::MenuItem("RayCast At Plane"))
			{
				OpenRayCastAtPlaneWindow();
			}

			if (ImGui::MenuItem("Verify UUIDs are unique"))
			{
				VerifyUUIDs();
			}

			/*
			if (ImGui::MenuItem("Randomize all UUIDs"))
			{
				for (auto& g : GetActiveScene()->GetGameObjects())
				{
					g->myUUID = UUID();
				}

				LOG_INFO(LogType::Editor) << "Done!";
			}
			*/

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

	if (myShowStyleEditor)
	{
		if (ImGui::Begin("Style Editor", &myShowStyleEditor))
		{
			ImGui::ShowStyleEditor();
		}
		ImGui::End();
	}


	if (myOpenNewScenePopup)
	{
		myOpenNewScenePopup = false;
		ImGui::OpenPopup("New Scene");
		memset(mySceneNameBuffer, 0, sizeof(mySceneNameBuffer));
		mySelectedSceneTypeId = SceneRegistry::GetIdFromTypeName("Default");
	}

	const auto flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize
		| ImGuiWindowFlags_NoSavedSettings;

	auto& io = ImGui::GetIO();
	ImVec2 pos(io.DisplaySize.x / 2.0f, io.DisplaySize.y / 2.0f);
	ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("New Scene", nullptr, flags))
	{
		ImGui::InputText("Scene Name", mySceneNameBuffer, sizeof(mySceneNameBuffer));

		if (ImGui::BeginCombo("Scene Type",
			(mySelectedSceneTypeId == static_cast<size_t>(-1))
			? ""
			: SceneRegistry::GetSceneTypeFromID(mySelectedSceneTypeId)->GetSceneTypeName().c_str()))
		{
			for (const auto& [id, type] : SceneRegistry::GetAllSceneTypes())
			{
				if (ImGui::Selectable(
					type->GetSceneTypeName().c_str(), id == mySelectedSceneTypeId))
				{
					mySelectedSceneTypeId = id;
				}
			}

			ImGui::EndCombo();
		}

		if (ImGui::Button("Create"))
		{
			const std::string sceneName = mySceneNameBuffer;

			if (!sceneName.empty())
			{
				CreateNewScene(sceneName);
			}
			else
			{
				ImGui::TextColored(ImVec4(1, 0, 0, 1), "Must have a have!");
			}
		}

		if (ImGui::Button("Back"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	// ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_Always);

	auto color = ImGui::GetStyleColorVec4(ImGuiCol_ChildBg);

	if (myMode == EditorMode::Playing)
	{
		color.x += 0.05f;
	}

	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::ColorConvertFloat4ToU32(color));

	if (!myIsSceneWindowFullscreen)
	{
		ZoneNamedN(zone4, "Editor::Update::AllWindowsUpdate", true);

		mySceneHierarchy->UpdateInternal(ts);
		myInspectorWindow->UpdateInternal(ts);
		for (auto& assetBrowser : myAssetBrowsers)
		{
			if (assetBrowser)
			{
				assetBrowser->UpdateInternal(ts);
			}
		}
		mySceneWindow->UpdateInternal(ts);
		myLogWindow->UpdateInternal(ts);

		for (auto& [name, extraWindow] : myExtraWindows)
		{
			if (extraWindow)
			{
				extraWindow->UpdateInternal(ts);

				if (!extraWindow->IsOpen())
				{
					// Null it then in another loop we remove it
					extraWindow = nullptr;
				}
			}
		}

		// If the gameobject selection was changed, make sure to modify the inspected gameobject
		if (GetActiveScene2().GetSelection().WasChanged())
		{
			// When we are editing a prefab, we must keep the InspectableGameObjectPrefab thingy
			// and only change internal gameobject within that prefab
			if (mySceneHierarchy->GetState() == SceneHierarchyWindow::State::DrawGameObjectPrefabHierarchy)
			{
				if (InspectableGameObjectPrefab* inspectablePrefab = dynamic_cast<InspectableGameObjectPrefab*>(myInspectorWindow->GetInspectable()))
				{
					inspectablePrefab->SetPrefabGameObject(GetActiveScene2().GetSelection().GetAtWeak(0).lock());
				}
			}
			else
			{
				myInspectorWindow->InspectObject(
					MakeOwned<InspectableSelection>(*this, *myInspectorWindow, GetActiveScene2().GetSelection()));
			}
		}
	}
	else
	{
		mySceneWindow->UpdateInternal(ts);
	}

	ImGui::PopStyleColor();

	// TODO: Do unique keypresses for each window that has focus?
	// Prevent keys from picking up when playing to avoid collisions
	if (myMode == EditorMode::Editing)
	{
		UpdateKeypresses();
	}

	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F5), true))
	{
		if (myMode == EditorMode::Editing)
		{
			ChangeMode(EditorMode::Playing);
		}
		else
		{
			ChangeMode(EditorMode::Editing);
		}
	}

	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F6), false))
	{
		myIsSceneWindowFullscreen = !myIsSceneWindowFullscreen;
	}

	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F4), false))
	{
		ExitEditor();
	}

	// Update the editor camera
	if (myMode == EditorMode::Editing)
	{
		if (myCameraComponent)
		{
			myCameraComponent->Execute(eEngineOrder::UPDATE);
		}

		if (myEditorCamMovement)
		{
			myEditorCamMovement->Execute(eEngineOrder::UPDATE);
		}
	}

	if (myMode == EditorMode::Editing)
	{
		if (GetActiveScene())
		{
			//const Mat4f projection = GetActiveScene()->GetMainCamera().GetProjectionMatrix();
			//const Mat4f view = Mat4f::GetFastInverse(GetActiveScene()->GetMainCamera().GetTransform().ToMatrix());

			//Mat4f trans;

			//ImGuizmo::DrawGrid(
			//	view.GetPointer(),
			//	projection.GetPointer(),
			//	trans.GetPointer(),
			//	10.f);
		}
	}

	for (auto& tool : myExternalTools)
	{
		tool();
	}

	GetActiveScene2().EndFrame();

	ImGui::End();

	myReturnedToEditorThisFrame = false;
}

void Engine::Editor::CloneAndSetScene(Shared<FolderScene> aFolderScene)
{
	PrepareSceneForEditing(*aFolderScene);

	// myEditingScene = std::move(aFolderScene);
	myEditingScene.SetScene(aFolderScene);
	myEditingScene.GetScene()->mySceneBeingEdited = true;
	
	if (myStaticSceneHandle.IsValid())
	{
		GetEngine().GetSceneManager().RemoveScene(myStaticSceneHandle);
	}
	
	myStaticSceneHandle = GetEngine().GetSceneManager().AddScene(aFolderScene);
}

bool Engine::Editor::LoadSaveData()
{
	myRecentScenes.clear();

	const std::string ourSavePathOld = "Dev\\EditorSave\\";

	if (std::filesystem::exists(ourSavePathOld))
	{
		// Move it to appdata
		const auto& destinationpath = GetEngine().GetPaths().GetPath(PathEnum::EditorSaveFolder);
		std::error_code e;

		for (auto& dir : std::filesystem::recursive_directory_iterator(ourSavePathOld))
		{
			FileIO::RemoveReadOnly(Path(dir));
		}

		// Copy to destination
		std::filesystem::copy(
			ourSavePathOld,
			destinationpath,
			std::filesystem::copy_options::recursive |
			std::filesystem::copy_options::overwrite_existing,
			e);

		if (e.value() != 0)
		{
			LOG_ERROR(LogType::Editor) << "Unable to copy the current path to the destination!";
			return false;
		}

		std::filesystem::remove_all(ourSavePathOld, e);

		if (e.value() != 0)
		{
			LOG_ERROR(LogType::Editor) << "Unable to remove old save path folder!";
			return false;
		}
	}

	const auto saveJsonPath = GetEngine().GetPaths().GetPath(PathEnum::EditorSaveFolder) / "EditorSaveData.json";
	std::ifstream file(saveJsonPath);

	if (!file.is_open())
	{
		LOG_ERROR(LogType::Resource) << "Failed to open " << saveJsonPath;
		return false;
	}

	const bool ignoreComments = true;
	const bool allowExceptions = false;
	auto j = nlohmann::json::parse(file, nullptr, allowExceptions, ignoreComments);

	// If failed to parse json file
	if (j.is_discarded())
	{
		LOG_ERROR(LogType::Resource) << "Failed to parse " << saveJsonPath;
		return false;
	}

	/*if (j.contains("AssetBrowser"))
	{
		myAssetBrowser->InitFromJson(j["AssetBrowser"]);
	}*/

	for (const std::string recentScenePath : j["Recent"])
	{
		if (std::filesystem::exists(recentScenePath))
		{
			myRecentScenes.push_back(recentScenePath);
		}
	}

	if (j.contains("Windows"))
	{
		if (j["Windows"].contains("AssetBrowsers"))
		{
			// Skip first one because its always enabled
			for (int i = 0; i < j["Windows"]["AssetBrowsers"].size(); ++i)
			{
				if (j["Windows"]["AssetBrowsers"][i].is_null())
				{
					continue;
				}

				myAssetBrowsers[i] = MakeOwned<AssetBrowserWindow>(*this, i);
				myAssetBrowsers[i]->Init(myResourceReferences);
				myAssetBrowsers[i]->InitFromJson(j["Windows"]["AssetBrowsers"][i]);
			}
		}
	}

	return true;
}

bool Engine::Editor::LoadSceneSpecificSaveData(const std::string aSceneFolderPathStem)
{
	const auto path = GetEngine().GetPaths().GetPath(PathEnum::EditorSaveFolder) / "Scenes" / (aSceneFolderPathStem + ".json");
	std::ifstream file(path);

	if (!file.is_open())
	{
		return false;
	}

	const bool ignoreComments = true;
	const bool allowExceptions = false;
	auto j = nlohmann::json::parse(file, nullptr, allowExceptions, ignoreComments);

	// If failed to parse json file
	if (j.is_discarded())
	{
		LOG_ERROR(LogType::Resource) << "Failed to parse " << path;
		return false;
	}

	if (j.contains("RuntimeScene"))
	{
		myRuntimeScene.InitFromJson(j["RuntimeScene"]);
	}

	if (j.contains("EditedScene"))
	{
		myEditingScene.InitFromJson(j["EditedScene"]);
	}

	if (j.contains("EditorCameraPos"))
	{
		if (!j["EditorCameraPos"][0].is_null() || !j["EditorCameraPos"][1].is_null() || !j["EditorCameraPos"][2].is_null())
		{
			myEditorCameraPosition.x = j["EditorCameraPos"][0];
			myEditorCameraPosition.y = j["EditorCameraPos"][1];
			myEditorCameraPosition.z = j["EditorCameraPos"][2];
		}
	}

	if (j.contains("EditorCameraRot"))
	{
		myEditorCameraRotation.myVector.x = j["EditorCameraRot"][0];
		myEditorCameraRotation.myVector.y = j["EditorCameraRot"][1];
		myEditorCameraRotation.myVector.z = j["EditorCameraRot"][2];
		myEditorCameraRotation.myW = j["EditorCameraRot"][3];
	}

	if (j.contains("EditorCameraRotationForComponent"))
	{
		myEditorCameraRotationForComponent.x = j["EditorCameraRotationForComponent"][0];
		myEditorCameraRotationForComponent.y = j["EditorCameraRotationForComponent"][1];
		myEditorCameraRotationForComponent.z = j["EditorCameraRotationForComponent"][2];
	}

	if (j.contains("EditorCamSpeed"))
	{
		myEditorCamSpeed = j["EditorCamSpeed"];
	}

	return true;
}

bool Engine::Editor::SaveSceneSpecificSaveData() const
{
	nlohmann::json j;

	j["EditorCameraPos"][0] = myEditorCameraPosition.x;
	j["EditorCameraPos"][1] = myEditorCameraPosition.y;
	j["EditorCameraPos"][2] = myEditorCameraPosition.z;

	j["EditorCameraRot"][0] = myEditorCameraRotation.myVector.x;
	j["EditorCameraRot"][1] = myEditorCameraRotation.myVector.y;
	j["EditorCameraRot"][2] = myEditorCameraRotation.myVector.z;
	j["EditorCameraRot"][3] = myEditorCameraRotation.myW;

	j["EditorCameraRotationForComponent"][0] = myEditorCameraRotationForComponent.x;
	j["EditorCameraRotationForComponent"][1] = myEditorCameraRotationForComponent.y;
	j["EditorCameraRotationForComponent"][2] = myEditorCameraRotationForComponent.z;

	j["EditorCamSpeed"] = myEditorCamMovement->GetSpeed();

	j["RuntimeScene"] = myRuntimeScene.ToJson();
	j["EditedScene"] = myEditingScene.ToJson();

	const auto scenesFolderPath = GetEngine().GetPaths().GetPath(PathEnum::EditorSaveFolder) / "Scenes";

	if (!std::filesystem::exists(scenesFolderPath))
	{
		std::filesystem::create_directory(scenesFolderPath);
	}

	const auto path = GetEngine().GetPaths().GetPath(PathEnum::EditorSaveFolder) / "Scenes" /
		(myEditingScene.GetScene()->GetFolderPath().GetStem().ToString() + ".json");

	FileIO::RemoveReadOnly(Path(path));

	std::ofstream file(path);

	if (file.is_open())
	{
		file << std::setw(4) << j;
		file.close();
	}
	else
	{
		LOG_ERROR(LogType::Engine) << "Failed to save editor save data, is it read only?";
		return false;
	}

	return true;
}

bool Engine::Editor::SaveToFile() const
{
	if (myEditingScene.GetScene())
	{
		SaveSceneSpecificSaveData();
	}

	nlohmann::json recentScenesJson = nlohmann::json::array();
	for (const auto& recentScenePath : myRecentScenes)
	{
		recentScenesJson.push_back(recentScenePath.ToString());
	}

	nlohmann::json j;

	j["Recent"] = recentScenesJson;

	nlohmann::json assetBrowsersJson = nlohmann::json::array();

	for (size_t i = 0; i < myAssetBrowsers.size(); ++i)
	{
		if (myAssetBrowsers[i] != nullptr)
		{
			assetBrowsersJson.push_back(myAssetBrowsers[i]->ToJson());
		}
		else
		{
			assetBrowsersJson.push_back(nlohmann::json());
		}
	}

	j["Windows"]["AssetBrowsers"] = assetBrowsersJson;

	const auto saveJsonPath = GetEngine().GetPaths().GetPath(PathEnum::EditorSaveFolder) / "EditorSaveData.json";

	FileIO::RemoveReadOnly(Path(saveJsonPath));

	std::ofstream file(saveJsonPath);

	if (file.is_open())
	{
		file << std::setw(4) << j;
		file.close();
	}
	else
	{
		LOG_ERROR(LogType::Engine) << "Failed to save editor save data, is it read only?";
		return false;
	}

	return true;
}

void Engine::Editor::AddExternalTool(const std::function<void(void)>& aTool)
{
	myExternalTools.push_back(aTool);
}

Engine::FolderScene* Engine::Editor::GetEditingScene()
{
	return myEditingScene.GetScene();
}

Engine::FolderScene* Engine::Editor::GetRuntimeScene()
{
	return myRuntimeScene.GetScene();
}

Engine::SceneBeingEdited& Engine::Editor::GetEditingScene2()
{
	return myEditingScene;
}

Engine::SceneBeingEdited& Engine::Editor::GetRuntimeScene2()
{
	return myRuntimeScene;
}

Engine::FolderScene* Engine::Editor::GetActiveScene()
{
	switch (myMode)
	{
	case EditorMode::Editing:
		return myEditingScene.GetScene();
	case EditorMode::Playing:
		return myRuntimeScene.GetScene();
	default:
		assert(false);
		break;
	};

	return nullptr;
}

Engine::SceneBeingEdited& Engine::Editor::GetActiveScene2()
{
	switch (myMode)
	{
	case EditorMode::Editing:
		return myEditingScene;
	case EditorMode::Playing:
		return myRuntimeScene;
	default:
		assert(false);
		break;
	};

	assert(false);

	throw;
}

void Engine::Editor::PushCommand(Owned<EditorCommand> aCommand)
{
	myCommands.push_back(std::move(aCommand));
}

void Engine::Editor::FinishCommand()
{
	assert(!myCommands.empty());

	// If more than 1 command, turn it into a composite
	if (myCommands.size() > 1)
	{
		auto composite = MakeOwned<EditorCommandComposite>();

		for (auto& command : myCommands)
		{
			composite->AddCommand(std::move(command));
		}

		myEditorCommandManager.DoCommand(std::move(composite));
	}
	else
	{
		myEditorCommandManager.DoCommand(std::move(myCommands.front()));
	}

	myCommands.clear();
}

Owned<Engine::EditorCommand> Engine::Editor::CreateCommand_AddGameObject(const Shared<GameObject>& aGameObjects)
{
	return MakeOwned<AddGameObjectCommand>(*GetActiveScene(), aGameObjects);
}

Owned<Engine::EditorCommand> Engine::Editor::CreateCommand_SelectGameObjects(const std::vector<Shared<GameObject>>& aGameObjects)
{
	Selection newSelection;
	for (auto& g : aGameObjects)
	{
		newSelection.SelectGameObject(g);
	}

	return MakeOwned<SetSelectionGameObjectsCommand>(GetActiveScene2().GetSelection(), newSelection);
}

Owned<Engine::EditorCommand> Engine::Editor::CreateCommand_SelectGameObjectsAppend(const std::vector<Shared<GameObject>>& aGameObjects)
{
	Selection newSelection = GetActiveScene2().GetSelection();
	for (auto& g : aGameObjects)
	{
		newSelection.SelectGameObject(g);
	}
	return MakeOwned<SetSelectionGameObjectsCommand>(GetActiveScene2().GetSelection(), newSelection);
}

Owned<Engine::EditorCommand> Engine::Editor::CreateCommand_DeselectGameObjects(const std::vector<Shared<GameObject>>& aGameObjects)
{
	auto newSelection = GetActiveScene2().GetSelection();

	for (auto& g : aGameObjects)
	{
		newSelection.DeselectGameObject(g.get());
	}

	// Basically just a selection of NO gameobjects
	return MakeOwned<SetSelectionGameObjectsCommand>(GetActiveScene2().GetSelection(), newSelection);
}

Owned<Engine::EditorCommand> Engine::Editor::CreateCommand_RemoveGameObject(
	const Shared<GameObject>& aGameObjects)
{
	return MakeOwned<RemoveGameObjectCommand>(*GetActiveScene(), aGameObjects);
}

Owned<Engine::EditorCommand> Engine::Editor::CreateCommand_AddComponent(
	const Shared<GameObject>& aGameObject, Component* aComponent) const
{
	return MakeOwned<AddComponentCommand>(aGameObject, aComponent);
}

Owned<Engine::EditorCommand> Engine::Editor::CreateCommand_RemoveComponent(
	const Shared<GameObject>& aGameObject, Component* aComponent) const
{
	return MakeOwned<RemoveComponentCommand>(aGameObject, aComponent);
}

Owned<Engine::EditorCommand> Engine::Editor::CreateCommand_MoveGameObject(
	const Shared<GameObject>& aGameObject,
	const int aTargetIndex)
{
	return MakeOwned<MoveGameObjectCommand>(*GetActiveScene(), aGameObject, aTargetIndex);
}

Owned<Engine::EditorCommand> Engine::Editor::CreateCommand_TransformGameObject(
	const Shared<GameObject>& aGameObject,
	const Mat4f& aStartTransform,
	const Mat4f& aEndTransform)
{
	return MakeOwned<TransformCommand>(aGameObject, aStartTransform, aEndTransform);
}

Engine::EditorMode Engine::Editor::GetMode() const
{
	return myMode;
}

void Engine::Editor::ChangeMode(const EditorMode aMode)
{
	switch (aMode)
	{
	case EditorMode::Editing:
		GetEngine().GetTimeSystem().ClearSceneTimers(); // UGLY

		// free the runtime scene, because it will not be visible
		myRuntimeScene.SetScene(nullptr);
		GetEngine().GetSceneManager().RemoveScene(myRuntimeSceneHandle);
		AudioManager::GetInstance()->StopAll();
		// Reset the state for the scene, e.g. selection
		myRuntimeScene.Reset();
		break;

	case EditorMode::Playing:
	{
		GetEngine().GetTimeSystem().ClearSceneTimers(); // UGLY
		LOG_INFO(LogType::AIPlacement) << "Game Restarted <--------------------------------------------->";
		if (myEditingScene.GetScene() == nullptr)
		{
			LOG_INFO(LogType::Editor) << "Cannot play, not scene being edited";
			return;
		}

		const auto scene = std::shared_ptr<FolderScene>(myEditingScene.GetScene()->Clone());

		PrepareSceneForPlaying(*scene);

		myRuntimeScene.SetScene(scene);
		myRuntimeSceneHandle = GetEngine().GetSceneManager().AddScene(scene);
		break;
	}

	default:
		assert(false);
		break;
	}

	// We need to close the windows that have been created as part of the playing mode
	// due to having references to stuff in the playing scene
	for (auto& [_, extraWindow] : myExtraWindows)
	{
		if (extraWindow == nullptr)
		{
			continue;
		}

		extraWindow->OnEditorChangeMode(aMode);
	}

	myMode = aMode;
}

Engine::InspectorWindow& Engine::Editor::GetInspectorWindow()
{
	return *myInspectorWindow;
}

Engine::AssetBrowserWindow& Engine::Editor::GetAssetBrowserWindow()
{
	return *myAssetBrowsers[0];
}

Engine::SceneHierarchyWindow& Engine::Editor::GetSceneHierarchyWindow()
{
	return *mySceneHierarchy;
}

Engine::LogWindow& Engine::Editor::GetLogWindow()
{
	return *myLogWindow;
}

Engine::SceneWindow& Engine::Editor::GetSceneWindow()
{
	return *mySceneWindow;
}

void Engine::Editor::OpenVisualGraphNodeEditorOfExistingInstanec(GraphInstance* aGraphInstance, const Path& aSavePath)
{
	auto visualGraphWindow = MakeOwned<VisualScriptWindow>(*this);
	visualGraphWindow->InitExistingInstance(aGraphInstance, aSavePath);
	visualGraphWindow->BringToFront();

	myExtraWindows[visualGraphWindow->GetName()] = std::move(visualGraphWindow);
}

void Engine::Editor::OpenVisualGraphNodeEditorNewInstance(const Path& aSavePath)
{
	// Open in graph manager
	auto visualGraphResource
		= GetEngine().GetResourceManager().CreateRef<VisualScriptResource>(aSavePath.ToString());

	visualGraphResource->Load();

	std::unique_ptr<GraphInstance> graphInstance(visualGraphResource->Get().CreateInstance(nullptr));

	if (graphInstance == nullptr)
	{
		LOG_ERROR(LogType::Engine) << "Unable to create graph instance: " << aSavePath;
		return;
	}

	auto visualGraphWindow = MakeOwned<VisualScriptWindow>(*this);
	visualGraphWindow->InitNewInstance(std::move(graphInstance), aSavePath);
	visualGraphWindow->BringToFront();

	myExtraWindows[visualGraphWindow->GetName()] = std::move(visualGraphWindow);
}

void Engine::Editor::OpenAnimationEditorOfExistingInstance(AnimationMachine* aMachine, const Path& aSavePath)
{
	auto animNodeEditor = MakeOwned<AnimationNodeEditorWindow>(*this);
	animNodeEditor->InitExistingMachine(aMachine, aSavePath);
	animNodeEditor->BringToFront();

	myExtraWindows[animNodeEditor->GetName()] = std::move(animNodeEditor);
}

void Engine::Editor::OpenAnimationEditorNewInstanceOfMachine(const Path& aPath)
{
	auto animNodeEditor = MakeOwned<AnimationNodeEditorWindow>(*this);

	auto myAnimationResource = GetEngine().GetResourceManager().CreateRef<AnimationStateMachineResource>(aPath.ToString());
	myAnimationResource->Load();

	auto machine = MakeOwned<AnimationMachine>();

	if (myAnimationResource->IsValid())
	{
		if (!machine->InitFromJson2(myAnimationResource->Get().GetJson()))
		{
			LOG_ERROR(LogType::Editor) << "Failed to create animations for " << myAnimationResource->GetPath();
			return;
		}
	}

	animNodeEditor->InitNewMachineInstance(std::move(machine), aPath);
	animNodeEditor->BringToFront();

	myExtraWindows[animNodeEditor->GetName()] = std::move(animNodeEditor);
}

void Engine::Editor::OpenCurveEditor(const AnimationCurveRef& aCurve)
{
	auto animationCurveEditorWindow = MakeOwned<AnimationCurveEditorWindow>(*this);
	animationCurveEditorWindow->Init(aCurve);
	animationCurveEditorWindow->BringToFront();

	myExtraWindows[animationCurveEditorWindow->GetName()] = std::move(animationCurveEditorWindow);
}

void Engine::Editor::OpenImportWindow(
	const ResourceImportState aImportState,
	const std::filesystem::path& aResourcePath)
{
	auto importWindow = MakeOwned<ImportWindow>(*this);
	importWindow->EnterImportState(aImportState, aResourcePath);
	importWindow->BringToFront();

	myExtraWindows[importWindow->GetName()] = std::move(importWindow);
}

void Engine::Editor::ShowMaterialInInspector(const std::filesystem::path& aMaterialPath)
{
	auto materialRef = GResourceManager->CreateRef<MaterialResource>(aMaterialPath.string());

	materialRef->Load();

	if (!materialRef || !materialRef->IsValid())
	{
		LOG_ERROR(LogType::Resource) << "Unable to open " << aMaterialPath;
		return;
	}

	myInspectorWindow->BringToFront(false);
	myInspectorWindow->InspectObject(MakeOwned<InspectableMaterial>(*this, *myInspectorWindow, materialRef));
}

void Engine::Editor::ShowGameObjectPrefabInInspector(const std::filesystem::path& aMaterialPath)
{
	auto gameObjectPrefabRef = GResourceManager->CreateRef<GameObjectPrefabResource>(aMaterialPath.string());

	gameObjectPrefabRef->Load();

	if (!gameObjectPrefabRef || !gameObjectPrefabRef->IsValid())
	{
		LOG_ERROR(LogType::Resource) << "Unable to open " << aMaterialPath;
		return;
	}

	myInspectorWindow->BringToFront(false);
	myInspectorWindow->InspectObject(MakeOwned<InspectableGameObjectPrefab>(*this, *myInspectorWindow, gameObjectPrefabRef));

	mySceneHierarchy->ViewGameObjectPrefab(gameObjectPrefabRef);
}

void Engine::Editor::OpenImportWindow(const ResourceImportState aImportState)
{
	auto importWindow = MakeOwned<ImportWindow>(*this);
	importWindow->EnterImportState(aImportState);
	importWindow->BringToFront();

	myExtraWindows[importWindow->GetName()] = std::move(importWindow);
}

void Engine::Editor::OpenRayCastAtPlaneWindow()
{
	auto rayCastAtPlaneWindow = MakeOwned<RayCastAtPlaneWindow>(*this);
	rayCastAtPlaneWindow->BringToFront();

	myExtraWindows[rayCastAtPlaneWindow->GetName()] = std::move(rayCastAtPlaneWindow);
}

void Engine::Editor::OpenMakeRetailBuildWindow()
{
	auto makeRetailBuildWindow = MakeOwned<MakeRetailBuildWindow>(*this);
	makeRetailBuildWindow->BringToFront();

	myExtraWindows[makeRetailBuildWindow->GetName()] = std::move(makeRetailBuildWindow);
}

void Engine::Editor::CloseExtraWindow(const std::string& aName)
{
	auto result = myExtraWindows.find(aName);

	if (result != myExtraWindows.end())
	{
		result->second = nullptr;
		// myExtraWindows.erase(aName);
	}
	else
	{
		LOG_WARNING(LogType::Animation) << "Tried closing window: " << aName << ", but was not found";
	}
}

Engine::LogWindow& Engine::Editor::Info() const
{
	return myLogWindow->Info();
}

Engine::LogWindow& Engine::Editor::Warning() const
{
	return myLogWindow->Warning();
}

Engine::LogWindow& Engine::Editor::Error() const
{
	return myLogWindow->Error();
}

bool Engine::Editor::DrawSearchResourcePopup(
	const std::vector<std::string>& aExtension,
	std::filesystem::path& aSelectedPathOut) const
{
	bool result = false;

	if (ImGui::BeginPopup("SearchResourcePopup"))
	{
		static char pathSearchBuffer[255] = { };

		static std::vector<std::filesystem::path> resourcePathsInList;

		bool changedSearch = false;

		// focus on the text input
		if (myShouldFocusOnAddComponentInput)
		{
			myShouldFocusOnAddComponentInput = false;

			ImGui::SetKeyboardFocusHere();

			// update the values the first time
			changedSearch = true;
		}

		if (ImGui::InputTextWithHint("##", "Search resource", pathSearchBuffer, sizeof(pathSearchBuffer)))
		{
			changedSearch = true;

			// when we change search, remove the selection
			// mySelectedResourceIndex = -1;
		}

		const std::string pathSearchBufferStr = pathSearchBuffer;

		if (pathSearchBufferStr.empty())
		{
			// All resources, not search atm
			if (changedSearch)
			{
				resourcePathsInList.clear();

				for (const auto& entry : myAssetBrowsers[0]->GetCachedAssetsFilePaths())
				{
					if (entry.myIsDirectory)
					{
						continue;
					}

					if (entry.myPath.has_extension())
					{
						std::string extension = entry.myPath.extension().string();

						for (auto& c : extension)
						{
							c = tolower(c);
						}

						if (std::find(aExtension.begin(), aExtension.end(), extension) != aExtension.end())
						{
							resourcePathsInList.push_back(entry.myPath);
						}
					}
				}

				if (!resourcePathsInList.empty())
				{
					mySelectedResourceIndex = 0;
				}
			}
		}
		else
		{
			if (changedSearch)
			{
				resourcePathsInList.clear();

				// Search using keys separated by spaces
				const std::vector<std::string> searchKeys = StringUtilities::Split(pathSearchBufferStr, " ");

				for (const auto& entry : myAssetBrowsers[0]->GetCachedAssetsFilePaths())
				{
					if (entry.myIsDirectory)
					{
						continue;
					}

					if (entry.myPath.has_extension())
					{
						std::string extension = entry.myPath.extension().string();

						for (auto& c : extension)
						{
							c = tolower(c);
						}

						if (std::find(aExtension.begin(), aExtension.end(), extension) != aExtension.end())
						{
							const auto stem = entry.myPath.stem().string();

							bool allKeysMatch = true;

							for (const auto& searchKey : searchKeys)
							{
								// Lower case string search, good? bad? not sure
								auto findResult = std::search(
									stem.begin(),
									stem.end(),
									searchKey.begin(),
									searchKey.end(),
									[](char c1, char c2)
									{
										return toupper(c1) == toupper(c2);
									});

								if (findResult == stem.end())
								{
									allKeysMatch = false;
								}
							}

							if (allKeysMatch)
							{
								resourcePathsInList.push_back(entry.myPath);
							}
						}
					}
				}

				if (!resourcePathsInList.empty())
				{
					mySelectedResourceIndex = 0;
				}
			}
		}

		if (ImGui::BeginListBox("##dsa"))
		{
			for (int i = 0; i < resourcePathsInList.size(); ++i)
			{
				const auto& path = resourcePathsInList[i];

				if (ImGui::Selectable(path.stem().string().c_str(), i == mySelectedResourceIndex))
				{
					memset(pathSearchBuffer, 0, sizeof(pathSearchBuffer));
					aSelectedPathOut = path;
					ImGui::CloseCurrentPopup();

					result = true;
					break;
				}

				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip(path.string().c_str());
				}
			}

			ImGui::EndListBox();
		}

		if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow)))
		{
			--mySelectedResourceIndex;
			mySelectedResourceIndex = std::max(mySelectedResourceIndex, 0);
		}

		if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow)))
		{
			++mySelectedResourceIndex;
			mySelectedResourceIndex = std::min(mySelectedResourceIndex, static_cast<int>(resourcePathsInList.size()) - 1);
		}

		if (ImGui::IsKeyPressed(static_cast<int>(CU::KeyCode::Enter)) && resourcePathsInList.size())
		{
			memset(pathSearchBuffer, 0, sizeof(pathSearchBuffer));
			aSelectedPathOut = resourcePathsInList[mySelectedResourceIndex];
			ImGui::CloseCurrentPopup();

			result = true;
		}

		ImGui::EndPopup();
	}

	return result;
}

void Engine::Editor::DrawFileMenu()
{
	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::MenuItem("New Scene"))
		{
			myOpenNewScenePopup = true;
		}

		if (ImGui::MenuItem("Open Scene"))
		{
			const auto paths = FileIO::BrowseFileDialog({ L"scene" }, L"scene");
			assert(paths.size() <= 1);

			if (!paths.empty())
			{
				OpenScene(paths.front());
			}
		}

		if (ImGui::MenuItem("Save Scene", "Ctrl+S", false,
			myMode == EditorMode::Editing &&
			myEditingScene.GetScene() != nullptr))
		{
			SaveScene();
		}

		if (ImGui::BeginMenu("Recent Scenes"))
		{
			for (auto& scenePath : GetRecentScenes())
			{
				if (ImGui::MenuItem(scenePath.ToString().c_str()))
				{
					OpenScene(scenePath);
				}
			}

			ImGui::EndMenu();
		}

		if (ImGui::MenuItem("Make Retail Build"))
		{
			OpenMakeRetailBuildWindow();
		}

		ImGui::EndMenu();
	}
}

void Engine::Editor::DrawViewMenu()
{
	if (ImGui::BeginMenu("View"))
	{
		if (ImGui::MenuItem("Toggle fullscreen", "F6"))
		{
			myIsSceneWindowFullscreen = !myIsSceneWindowFullscreen;
		}

		if (ImGui::MenuItem("Enter/Exit editor", "F4"))
		{
			ExitEditor();
		}

		ImGui::EndMenu();
	}
}

void Engine::Editor::DrawToolsMenu()
{
	if (ImGui::BeginMenu("Tools"))
	{
		if (ImGui::MenuItem("Bake Destructables"))
		{

		}


		ImGui::EndMenu();
	}
}

void Engine::Editor::PrepareSceneForEditing(Scene& aScene)
{
	// This caused crash sometimes because it does not copy correctly atm?
	aScene.SetIsEditorScene(true);

	// Set set manually to avoid going into CreatingGameObjects
	// state when changing to this scene
	aScene.myInitState = Scene::InitState::None;
	aScene.myState = Scene::State::Ready;

	// TODO: By making the camera a gameobject in the scene, it will get saved as well
	// maybe find a better solution and maybe just allocate them all in here privately
	// Create the editor camera
	// myEditorCamera = myGameObjectManager.AddGameObject<GameObject>();
	myEditorCamera = aScene.AddGameObject<GameObject>();
	myEditorCamera->SetName("EditorCamera");
	// NOTE(filip): making it from unreal to stop it from being serialized
	myEditorCamera->SetIsFromUnreal(true);
	myEditorCamera->GetTransform().SetPosition(myEditorCameraPosition);
	myEditorCamera->GetTransform().SetRotation(myEditorCameraRotation);
	myCameraComponent = myEditorCamera->AddComponent<CameraComponentNew>();
	myEditorCamMovement = myEditorCamera->AddComponent<EditorCameraMovement>();

	// Must do this, otherwise the camera snaps to 0,0 rotation when starting to move
	myEditorCamMovement->SetCurrentRotation(myEditorCameraRotationForComponent);
	myEditorCamMovement->SetSpeed(myEditorCamSpeed);

	aScene.SetMainCamera(myEditorCamera);
}

void Engine::Editor::PrepareSceneForPlaying(Scene& aScene)
{
	aScene.SetIsEditorScene(false);

	// Set set manually to because we previously set it to ready when cloning it
	// otherwise we'd never actually created the proper gameobjects for the scene in runtime code
	aScene.myInitState = Scene::InitState::RuntimeInit;
	aScene.myState = Scene::State::NotReady;

	aScene.myIsEnabled = true;

	// Must remove the editor camera in play mode
	auto editorCam = aScene.FindGameObject("EditorCamera");
	aScene.myGameObjectManager.RemoveGameObject(editorCam);
}

bool Engine::Editor::CreateNewScene(const std::string& aSceneName)
{
	const auto scenePath = "Assets\\Scenes\\" + aSceneName;

	if (!std::filesystem::exists(scenePath))
	{
		std::filesystem::create_directory(scenePath);

		AddScenePathToRecent(scenePath + "\\Scene.scene");
		SaveToFile();

		FolderScene* sceneType = SceneRegistry::GetSceneTypeFromID(mySelectedSceneTypeId);

		if (sceneType)
		{
			FolderScene* newFolderScene = sceneType->Clone();
			newFolderScene->SetFolderPath(scenePath);
			newFolderScene->Init();
			newFolderScene->myName = aSceneName;
			newFolderScene->myTypeId = sceneType->myTypeId;
			newFolderScene->myTypeName = sceneType->myTypeName;

			{
				auto postProcess = newFolderScene->AddGameObject<GameObject>();
				postProcess->SetName("Post Process");
				postProcess->AddComponent<PostProcessComponent>();
			}

			// Create Default Environment Light
			{
				auto environmentLightGameObject = newFolderScene->AddGameObject<GameObject>();
				environmentLightGameObject->SetName("EnvironmentLight");
				EnvironmentLightComponent* envComp = environmentLightGameObject->AddComponent<EnvironmentLightComponent>(
					GetEngine().GetPaths().GetPathString(PathEnum::PlaceholderCubemap));

				auto myCubemap = GResourceManager->CreateRef<TextureResource>(
					GetEngine().GetPaths().GetPathString(PathEnum::PlaceholderCubemap));

				myCubemap->Load();

				if (!myCubemap->IsValid())
				{
					assert(false);
				}

				envComp->SetCubemap1(myCubemap);
				envComp->InitLight();
			}

			// Create the main camera
			{
				auto cameraGameObject = newFolderScene->AddGameObject<GameObject>();
				cameraGameObject->SetName("MainCamera");
				auto cameraComponentNew = cameraGameObject->AddComponent<CameraComponentNew>();
				cameraGameObject->AddComponent<BasicMovementComponentNew>();
				cameraGameObject->AddComponent<CameraShakeComponent>();
			}

			newFolderScene->myUICamera = GetEngine().GetCameraFactory().CreateUICamera();

			/*
			auto newFolderSceneShared = std::shared_ptr<FolderScene>(newFolderScene);
			myRuntimeScene.SetScene(newFolderSceneShared);
			GetEngine().GetSceneManager().AddScene(ourRuntimeSceneName, newFolderSceneShared);
			*/

			SaveScene(*newFolderScene);

			OpenScene(newFolderScene->GetFolderPath().AppendPath("Scene.scene"));
		}
		else
		{
			const auto scene = MakeShared<FolderScene>();
			scene->SetFolderPath(scenePath);
			scene->Init();
		}

		ImGui::CloseCurrentPopup();
	}
	else
	{
		LOG_ERROR(LogType::Editor) << "Scene already exists";
		return false;
	}

	return true;
}

void Engine::Editor::UpdateMoveToCameraLerp()
{
	if (myIsMovingCamToSelection)
	{
		myMoveToSelectionDownTimer -= Time::DeltaTime;

		if (myMoveToSelectionDownTimer <= 0.f)
		{
			myIsMovingCamToSelection = false;
		}

		auto lerpedPos = Math::Lerp(
			myStartCamPos,
			GetActiveScene2().GetSelection().CalculateCenterTransform().GetPositionLocal(),
			Math::InverseLerp(myMoveToSelectionDurationSeconds, 0.f, myMoveToSelectionDownTimer));

		myCameraComponent->GetGameObject()->GetTransform().SetPosition(lerpedPos);
	}
}

void Engine::Editor::UpdateKeypresses()
{
	const auto& input = GetEngine().GetInputManager();

	const bool c = input.IsKeyDown(CU::KeyCode::C);
	const bool v = input.IsKeyDown(CU::KeyCode::V);
	const bool z = input.IsKeyDown(CU::KeyCode::Z);
	const bool y = input.IsKeyDown(CU::KeyCode::Y);
	const bool ctrl = input.IsKeyPressed(CU::KeyCode::LeftControl);

	if (ctrl && z)
	{
		myEditorCommandManager.Undo();
	}

	if (ctrl && y)
	{
		myEditorCommandManager.Redo();
	}

	if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_LeftControl)))
	{
		if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_S), false))
		{
			// We must save all of them to ensure nothing is left unsaved because
			// otherwise the user might think they save, but actually did not
			mySceneHierarchy->Save();
			myInspectorWindow->Save();
			for (auto& assetBrowser : myAssetBrowsers)
			{
				if (assetBrowser)
				{
					assetBrowser->Save();
				}
			}
			mySceneWindow->Save();
			myLogWindow->Save();

			// Save all the other open windows as well
			for (auto& window : myExtraWindows)
			{
				if (window.second)
				{
					window.second->Save();
				}
			}

			// Must save scene last
			SaveScene();
		}
	}
}

void Engine::Editor::SaveScene()
{
	if (myMode == EditorMode::Playing)
	{
		return;
	}

	if (myEditingScene.GetScene() == nullptr)
	{
		return;
	}

	SaveScene(*myEditingScene.GetScene());

	mySceneWindow->ShowFeedback();
}

void Engine::Editor::SaveScene(FolderScene& aFolderScene)
{
	const auto& folderPath = aFolderScene.GetFolderPath();

	// Save the scene metadata
	{
		nlohmann::json j;

		j["TypeName"] = aFolderScene.myTypeName;
		j["TypeId"] = aFolderScene.myTypeId;

		std::ofstream file(folderPath.AppendPath("Scene.meta").ToWString());

		if (file.is_open())
		{
			file << std::setw(4) << j;
			file.close();
			Info() << "Successfully saved the scene!";
		}
		else
		{
			Error() << "Failed to save scene, is it read only?";
		}
	}

	SceneSerializerJson jsonSerializer;

	jsonSerializer.Serialize(aFolderScene, folderPath.AppendPath("Scene.scene"));

	// Visit the scene with a resource ref reflector to find all the resources being used
	// and save them to the json file
	ResourceRefReflector resRefReflector;

	aFolderScene.myGameObjectManager.ReflectGameObjects(resRefReflector);
	aFolderScene.myGameObjectManager.ReflectAllGameObjectComponents(resRefReflector);

	resRefReflector.GetResourceRefs().SaveToJson(folderPath.AppendPath("SceneResourceRefs.json"));
}

void Engine::Editor::DrawEditMenu()
{
	if (ImGui::BeginMenu("Edit"))
	{
		if (ImGui::MenuItem("Play", "F5", false, myMode == EditorMode::Editing))
		{
			ChangeMode(EditorMode::Playing);
		}

		if (ImGui::MenuItem("Stop", "F5", false, myMode == EditorMode::Playing))
		{
			ChangeMode(EditorMode::Editing);
		}

		ImGui::EndMenu();
	}
}

void Engine::Editor::AddScenePathToRecent(const Path& aScenePath)
{
	for (size_t i = 0; i < myRecentScenes.size(); ++i)
	{
		if (myRecentScenes[i] == aScenePath)
		{
			// If found, we only move it to the top
			const Path pathCopy = myRecentScenes[i];
			myRecentScenes.erase(myRecentScenes.begin() + i);
			myRecentScenes.insert(myRecentScenes.begin(), pathCopy);

			return;
		}
	}

	myRecentScenes.insert(myRecentScenes.begin(), aScenePath);
}

void Engine::Editor::ReturnToEditor()
{
	myReturnedToEditorThisFrame = true;

	SetIsEnabled(true);

	myEditingScene.GetScene()->SetIsEnabled(true);

	GetEngine().GetSceneManager().SetMainScene(GetEngine().GetEditorSceneHandle());
}

void Engine::Editor::ExitEditor()
{
	if (myEditingScene.GetScene() == nullptr)
	{
		LOG_WARNING(LogType::Editor) << "Cannot exit editor without an active scene";
		return;
	}

	if (myReturnedToEditorThisFrame)
	{
		return;
	}

	SetIsEnabled(false);

	myEditingScene.GetScene()->SetIsEnabled(false);

	// If we are playing, then exit editor into this active scene
	if (myRuntimeScene.GetScene() == nullptr)
	{
		// If not playing, must start playing to keep the 
		// static scene from being changed when coming back into
		// the editor
		ChangeMode(EditorMode::Playing);
	}

	GetEngine().GetSceneManager().SetMainScene(myRuntimeSceneHandle);
}

void Engine::Editor::VerifyUUIDs()
{
	std::set<UUID> uuids;

	bool anyIssues = false;

	for (auto& g : GetActiveScene()->GetGameObjects())
	{
		const UUID uuid = g->GetUUID();

		if (uuids.find(uuid) != uuids.end())
		{
			LOG_WARNING(LogType::Editor) << "UUID: " << uuid << " already exists, GameObject: " << g->GetName();

			anyIssues = true;
		}
		else
		{
			uuids.insert(uuid);
		}
	}

	if (!anyIssues)
	{
		LOG_WARNING(LogType::Editor) << "All UUIDs are unique";
	}
}

void Engine::Editor::ResetDockingLayout()
{
	ImGuiID dockspace_id = ImGui::GetID(ourDockSpaceId);

	// Clear any previous layout
	ImGui::DockBuilderRemoveNode(dockspace_id);
	ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
	ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

	ImGuiID oppositeOfLeft;
	ImGuiID top;
	ImGuiID oppositeOfLeft3;
	ImGuiID rightOfBottomOfNode11;

	// split the dockspace into 2 nodes -- DockBuilderSplitNode takes in the following args in the following order
	//   window ID to split, direction, fraction (between 0 and 1), the final two setting let's us choose which id we want (which ever one we DON'T set as NULL, will be returned by the function)
	//                                                              out_id_at_dir is the id of the node in the direction we specified earlier, out_id_at_opposite_dir is in the opposite direction
	auto rightOfMainNode = ImGui::DockBuilderSplitNode(
		dockspace_id,
		ImGuiDir_Right,
		0.25f,
		nullptr,
		&oppositeOfLeft);

	auto bottomOfNode1 = ImGui::DockBuilderSplitNode(
		oppositeOfLeft,
		ImGuiDir_Down,
		0.25f,
		nullptr,
		&top);

	auto rightOfBottomOfNode1 = ImGui::DockBuilderSplitNode(
		top,
		ImGuiDir_Right,
		0.5f,
		nullptr,
		&rightOfBottomOfNode11);

	// Dock scene window into the main docking node
	ImGui::DockBuilderDockWindow(mySceneWindow->GetName().c_str(), dockspace_id);

	ImGui::DockBuilderDockWindow(myInspectorWindow->GetName().c_str(), rightOfMainNode);

	ImGui::DockBuilderDockWindow(myAssetBrowsers[0]->GetName().c_str(), bottomOfNode1);

	ImGui::DockBuilderDockWindow(mySceneHierarchy->GetName().c_str(), rightOfBottomOfNode1);

	ImGui::DockBuilderDockWindow(myLogWindow->GetName().c_str(), bottomOfNode1);

	ImGui::DockBuilderFinish(dockspace_id);
}

const std::vector<Path>& Engine::Editor::GetRecentScenes() const
{
	return myRecentScenes;
}

bool Engine::Editor::OpenScene(Path aPath)
{
	// go back to editing mode before opening new scene
	if (myMode == EditorMode::Playing)
	{
		ChangeMode(EditorMode::Editing);
	}

	// NOTE(filip): if opening a scene this way
	// it means that the scene cannot be subclassed which is have
	// previously relied upon

	// Read the meta file
	auto p = Path(aPath);
	p.ChangeExtension(".meta");
	std::ifstream file(p.ToWString());

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

	auto sceneType = SceneRegistry::GetSceneTypeFromID(j["TypeId"]);

	//const auto scene = MakeShared<FolderScene>();
	const auto scene = std::shared_ptr<FolderScene>(sceneType->Clone());
	scene->SetFolderPath(aPath.GetParentPath());
	scene->Init();

	myRuntimeScene.SetScene(scene);

	// set to null so we are re-cloning the newly opened 
	// scene in the update loop
	myEditingScene.SetScene(nullptr);

	myRuntimeSceneHandle = GetEngine().GetSceneManager().AddScene(scene);

	aPath.MakeRelativeTo(Path(std::filesystem::current_path()));
	AddScenePathToRecent(aPath);
	SaveToFile();

	return true;
}

Engine::EditorCameraMovement& Engine::Editor::GetEditorCamMovement()
{
	return *myEditorCamMovement;
}

void Engine::Editor::MoveEditorCamToSelection()
{
	myIsMovingCamToSelection = true;
	myMoveToSelectionDownTimer = myMoveToSelectionDurationSeconds;
	myStartCamPos = myCameraComponent->GetGameObject()->GetTransform().GetPosition();
}
