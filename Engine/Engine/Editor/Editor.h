#pragma once

#include "SceneBeingEdited.h"
#include "Selection/Selection.h"
#include "Command/EditorCommandManager.h"
#include "Engine/ResourceReferences.h"
#include "Engine/GameObject/GameObjectManager.h"
#include "Engine/Scene/Scene.h"
#include "EditorMode.h"
#include "EditorWindows/AssetBrowser/Importing/CreationStates/ResourceImportState.h"
#include "ImGuiHelper.h"

#include "imgui_internal.h"
#include "Engine/Editor/EditorWindows/Inspector/InspectorWindow.h"
#include "Engine/Scene/SceneManager.h"

class GraphInstance;
class GameObject;
class Transform;

namespace Engine
{
	class EditorCameraMovement;
	class EditorCommandComposite;
	class AnimationMachine;
	class AnimationNodeEditorWindow;
	class LogWindow;
	class EditorWindow;
	class SceneWindow;
	class VisualScriptWindow;
	class BasicMovementComponentNew;
	class CameraComponentNew;
	class AssetBrowserWindow;
	class InspectorWindow;
	class AnimationCurveEditorWindow;
	class Scene;
	class SceneHierarchyWindow;
	class FolderScene;
	class SceneManager;

	class Editor : public Scene
	{
	public:
		Editor();
		virtual ~Editor();

		virtual bool Init() override;

		virtual void Update(TimeStamp ts) override;

		virtual void CloneAndSetScene(Shared<FolderScene> aFolderScene);

		bool LoadSaveData();
		bool LoadSceneSpecificSaveData(const std::string aSceneFolderPathStem);
		bool SaveSceneSpecificSaveData() const;
		bool SaveToFile() const;

		void AddExternalTool(const std::function<void(void)>& aTool);

		// Returns nullptr if no scene is being edited
		FolderScene* GetEditingScene();
		FolderScene* GetRuntimeScene();

		SceneBeingEdited& GetEditingScene2();
		SceneBeingEdited& GetRuntimeScene2();

		// Returns the editing or runtime scene based on what mode we are in (playing/editing)
		FolderScene* GetActiveScene();
		SceneBeingEdited& GetActiveScene2();

		// Used to make several commands into one
		// Must call FinishCommand() after this
		void PushCommand(Owned<EditorCommand> aCommand);
		void FinishCommand();

		// Commands
		Owned<EditorCommand> CreateCommand_AddGameObject(const Shared<GameObject>& aGameObjects);
		Owned<EditorCommand> CreateCommand_SelectGameObjects(const std::vector<Shared<GameObject>>& aGameObjects);
		Owned<EditorCommand> CreateCommand_SelectGameObjectsAppend(const std::vector<Shared<GameObject>>& aGameObjects);
		Owned<EditorCommand> CreateCommand_DeselectGameObjects(const std::vector<Shared<GameObject>>& aGameObjects);
		Owned<EditorCommand> CreateCommand_RemoveGameObject(const Shared<GameObject>& aGameObjects);
		Owned<EditorCommand> CreateCommand_AddComponent(const Shared<GameObject>& aGameObject, Component* aComponent) const;
		Owned<EditorCommand> CreateCommand_RemoveComponent(const Shared<GameObject>& aGameObject, Component* aComponent) const;
		Owned<EditorCommand> CreateCommand_MoveGameObject(const Shared<GameObject>& aGameObject, const int aTargetIndex);
		Owned<EditorCommand> CreateCommand_TransformGameObject(
			const Shared<GameObject>& aGameObject,
			const Mat4f& aStartTransform,
			const Mat4f& aEndTransform);

		EditorMode GetMode() const;
		void ChangeMode(const EditorMode aMode);

		InspectorWindow& GetInspectorWindow();
		AssetBrowserWindow& GetAssetBrowserWindow();
		SceneHierarchyWindow& GetSceneHierarchyWindow();
		LogWindow& GetLogWindow();
		SceneWindow& GetSceneWindow();

		void OpenVisualGraphNodeEditorOfExistingInstanec(GraphInstance* aGraphInstance, const Path& aSavePath);
		void OpenVisualGraphNodeEditorNewInstance(const Path& aSavePath);
		void OpenAnimationEditorOfExistingInstance(AnimationMachine* aMachine, const Path& aSavePath);
		void OpenAnimationEditorNewInstanceOfMachine(const Path& aPath);
		void OpenCurveEditor(const AnimationCurveRef& aCurve);
		void OpenImportWindow(
			const ResourceImportState aImportState,
			const std::filesystem::path& aResourcePath);
		void OpenImportWindow(const ResourceImportState aImportState);
		void OpenRayCastAtPlaneWindow();
		void OpenMakeRetailBuildWindow();

		void ShowMaterialInInspector(const std::filesystem::path& aMaterialPath);
		void ShowGameObjectPrefabInInspector(const std::filesystem::path& aMaterialPath);

		void CloseExtraWindow(const std::string& aName);

		LogWindow& Info() const;
		LogWindow& Warning() const;
		LogWindow& Error() const;

		bool OpenScene(Path aPath);

		EditorCameraMovement& GetEditorCamMovement();

		void MoveEditorCamToSelection();

		// Returns true if value was changed
		template <typename T, typename ...Args>
		bool DrawReflectedResource(
			ResourceRef<T>& aValue,
			const std::string& aName,
			const std::string& aDragDropType,
			const std::string& aExtensionToSearchFor,
			const std::function<void()> aDrawRightClickMenuItems = nullptr,
			Args&... aArgs);

		template <typename T, typename ...Args>
		bool DrawReflectedResource(
			ResourceRef<T>& aValue,
			const std::string& aName,
			const std::vector<std::string>& aDragDropTypes,
			const std::vector<std::string>& aExtensionsToSearchFor,
			const std::function<void()> aDrawRightClickMenuItems = nullptr,
			Args&... aArgs);

		// Returns true if value was changed
		template <typename T, typename ...Args>
		bool DrawReflectedResourceValue(
			ResourceRef<T>& aValue,
			const std::string& aName,
			const std::string& aDragDropType,
			const std::string& aExtensionToSearchFor,
			const std::function<void()> aDrawRightClickMenuItems = nullptr,
			Args&... aArgs);

		template <typename T, typename ...Args>
		bool DrawReflectedResourceValue(
			ResourceRef<T>& aValue,
			const std::string& aName,
			const std::vector<std::string>& aDropDropTypes,
			const std::vector<std::string>& aExtensionsToSearchFor,
			const std::function<void()> aDrawRightClickMenuItems = nullptr,
			Args&... aArgs);

		bool DrawSearchResourcePopup(
			const std::vector<std::string>& aExtension,
			std::filesystem::path& aSelectedPathOut) const;

		const std::vector<Path>& GetRecentScenes() const;

		void ReturnToEditor();

	private:
		void DrawFileMenu();
		void DrawViewMenu();
		void DrawToolsMenu();
		void PrepareSceneForEditing(Scene& aScene);
		void PrepareSceneForPlaying(Scene& aScene);
		bool CreateNewScene(const std::string& aSceneName);
		void UpdateMoveToCameraLerp();

		void UpdateKeypresses();
		void SaveScene();
		void SaveScene(FolderScene& aFolderScene);

		void DrawEditMenu();
		void AddScenePathToRecent(const Path& aScenePath);

		void ExitEditor();
		void VerifyUUIDs();
		void ResetDockingLayout();

	private:
		std::vector<Owned<EditorCommand>> myCommands;

		std::vector<std::function<void(void)>> myExternalTools;

		Owned<SceneHierarchyWindow> mySceneHierarchy;
		Owned<InspectorWindow> myInspectorWindow;
		std::array<Owned<AssetBrowserWindow>, 4> myAssetBrowsers;
		Owned<SceneWindow> mySceneWindow;
		Owned<LogWindow> myLogWindow;

		// This is windows that are created due to some type of action
		// e.g. when clicking a .animation file it opens up the editor for it
		std::unordered_map<std::string, Owned<EditorWindow>> myExtraWindows;

		SceneBeingEdited myEditingScene;
		SceneBeingEdited myRuntimeScene;

		// Must be below SceneBeingEdited because it needs to be destroyed before the scene because
		// it holds Shared references to gameobject and keep them alive
		// otherwise, a crash occurs when closing the appliation
		EditorCommandManager myEditorCommandManager;

		bool myIsSceneWindowFullscreen = false;

		EditorMode myMode = EditorMode::Editing;

		bool myOpenNewScenePopup = false;
		char mySceneNameBuffer[255] = {};
		size_t mySelectedSceneTypeId = -1;

		// For the editor camera
		GameObject* myEditorCamera = nullptr;
		Vec3f myEditorCameraPosition;
		Quatf myEditorCameraRotation;
		Vec3f myEditorCameraRotationForComponent;
		float myEditorCamSpeed = 700.f;
		CameraComponentNew* myCameraComponent = nullptr;
		EditorCameraMovement* myEditorCamMovement = nullptr;

		ResourceReferences myResourceReferences;

		// Move to camera lerp
		bool myIsMovingCamToSelection = false;
		CU::Vector3f myStartCamPos;
		float myMoveToSelectionDownTimer = 0.f;
		float myMoveToSelectionDurationSeconds = 0.1f;


		// This works because we only ever have one popup visible at time
		static inline bool myShouldFocusOnAddComponentInput = false;
		static inline int mySelectedResourceIndex = -1;

		std::vector<Path> myRecentScenes;

		bool myShowStyleEditor = false;

		bool myReturnedToEditorThisFrame = false;

		bool myResetDockingLayout = false;

		// inline static std::string ourRuntimeSceneName = "RuntimeScene";
		// inline static std::string ourStaticSceneName = "StaticScene";

		SceneHandle myRuntimeSceneHandle;
		SceneHandle myStaticSceneHandle;

	public:
		// inline static std::string ourSceneName = "EditorScene";
		inline static const char* ourDockSpaceId = "MyDockSpace";

		static inline float ourAlignPercent = 0.3f;
	};

	template <typename T, typename ...Args>
	bool Editor::DrawReflectedResource(
		ResourceRef<T>& aValue,
		const std::string& aName,
		const std::vector<std::string>& aDragDropTypes,
		const std::vector<std::string>& aExtensionsToSearchFor,
		const std::function<void()> aDrawRightClickMenuItems,
		Args&... aArgs)
	{
		// Aligns the text vertially to be center in the table
		if (!aName.empty())
		{
			ImGui::AlignTextToFramePadding();
			ImGui::Text("%s", aName.c_str());
			ImGui::SameLine(ImGui::GetContentRegionAvailWidth() * ourAlignPercent);
		}

		return DrawReflectedResourceValue(
			aValue,
			aName,
			aDragDropTypes,
			aExtensionsToSearchFor,
			aDrawRightClickMenuItems,
			std::forward<Args&>(aArgs)...);
	}

	template <typename T, typename ...Args>
	bool Editor::DrawReflectedResource(
		ResourceRef<T>& aValue,
		const std::string& aName,
		const std::string& aDragDropType,
		const std::string& aExtensionToSearchFor,
		const std::function<void()> aDrawRightClickMenuItems,
		Args&... aArgs)
	{
		// Aligns the text vertially to be center in the table
		if (!aName.empty())
		{
			ImGui::AlignTextToFramePadding();
			ImGui::Text("%s", aName.c_str());
			ImGui::SameLine(ImGui::GetContentRegionAvailWidth() * ourAlignPercent);
		}

		return DrawReflectedResourceValue(
			aValue,
			aName,
			aDragDropType,
			aExtensionToSearchFor,
			aDrawRightClickMenuItems,
			std::forward<Args&>(aArgs)...);
	}

	template <typename T, typename ...Args>
	bool Editor::DrawReflectedResourceValue(
		ResourceRef<T>& aValue,
		const std::string& aName,
		const std::string& aDragDropType,
		const std::string& aExtensionToSearchFor,
		const std::function<void()> aDrawRightClickMenuItems,
		Args&... aArgs)
	{
		return DrawReflectedResourceValue(
			aValue,
			aName,
			std::vector<std::string> { aDragDropType },
			std::vector<std::string> { aExtensionToSearchFor },
			aDrawRightClickMenuItems,
			std::forward<Args&>(aArgs)...);
	}

	template <typename T, typename ...Args>
	bool Editor::DrawReflectedResourceValue(
		ResourceRef<T>& aValue,
		const std::string& aName,
		const std::vector<std::string>& aDragDropTypes,
		const std::vector<std::string>& aExtensionsToSearchFor,
		const std::function<void()> aDrawRightClickMenuItems,
		Args&... aArgs)
	{
		ImGui::PushID(&aValue);

		bool result = false;

		std::filesystem::path p;

		if (aValue && aValue->IsValid())
		{
			p = aValue->GetPath();
		}

		// Channel is like Z-order, widgets in higher channels are rendered above widgets in lower ones
		ImGui::GetWindowDrawList()->ChannelsSplit(2);
		ImGui::GetWindowDrawList()->ChannelsSetCurrent(1);

		// for some reason we need to offset to more to the right
		// otherwise does not line up with other aligned items
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 4.f);

		ImGui::AlignTextToFramePadding();
		if (ImGui::Selectable(
			p.stem().string().c_str(),
			false,
			0,
			ImVec2(ImGui::GetContentRegionAvailWidth() - 32.f, 0)))
		{
			if (!p.empty())
			{
				// Show in asset browser
				GetAssetBrowserWindow().GotoDirectory(p.parent_path());
				GetAssetBrowserWindow().SetSelectedFileByPath(p);
				GetAssetBrowserWindow().ScrollSelectedItemVisible();
			}
		}

		// Draw the border of the selectable ourself because it does not have a border by default
		//ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImGui::ColorConvertFloat4ToU32(ImGui::GetStyleColorVec4(ImGuiCol_Border)));

		//ImGui:Button

		ImGui::RenderFrame(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255, 0, 0, 0), true, ImGui::GetStyle().FrameRounding);


		// ImGui::RenderFrame();

		//void ImGui::RenderFrame(ImVec2 p_min, ImVec2 p_max, ImU32 fill_col, bool border, float rounding)
		//{
		//	ImGuiContext& g = *GImGui;
		//	ImGuiWindow* window = g.CurrentWindow;
		//	window->DrawList->AddRectFilled(p_min, p_max, fill_col, rounding);
		//	const float border_size = g.Style.FrameBorderSize;
		//	if (border && border_size > 0.0f)
		//	{
		//		window->DrawList->AddRect(p_min + ImVec2(1, 1), p_max + ImVec2(1, 1), GetColorU32(ImGuiCol_BorderShadow), rounding, 0, border_size);
		//		window->DrawList->AddRect(p_min, p_max, GetColorU32(ImGuiCol_Border), rounding, 0, border_size);
		//	}
		//}



		ImGui::GetWindowDrawList()->ChannelsSetCurrent(0);

		const auto rectMin = ImGui::GetItemRectMin();
		const auto rectMax = ImGui::GetItemRectMax();

		// Since Selectable() does not have a background color, I draw it myself
		ImGui::GetWindowDrawList()->AddRectFilled(
			rectMin,
			rectMax,
			ImGui::ColorConvertFloat4ToU32(ImVec4(0.2f, 0.2f, 0.2f, 1)),
			0);

		ImGui::GetWindowDrawList()->ChannelsMerge();

		if (ImGui::BeginPopupContextItem())
		{
			if (aDrawRightClickMenuItems)
			{
				aDrawRightClickMenuItems();
			}

			if (ImGui::MenuItem("Clear"))
			{
				aValue = {};
				result = true;
			}


			ImGui::EndPopup();
		}

		ImGui::SameLine();

		if (ImGui::IsItemHovered())
		{
			if (aValue && aValue->IsValid())
			{
				ImGui::SetTooltip(aValue->GetPath().c_str());
			}
		}

		for (const auto& aDragDropType : aDragDropTypes)
		{
			if (const ImGuiPayload* payload = ImGuiHelper::BeginDragDropTarget(aDragDropType.c_str()))
			{
				char path[255] = {};
				memcpy(path, payload->Data, payload->DataSize);

				aValue = GResourceManager->CreateRef<T>(path, std::forward<Args&>(aArgs)...);
				aValue->Load();

				result = true;
			}
		}

		if (ImGui::ImageButton(
			myInspectorWindow->GetSearchResourceIcon()->Get().GetSRV(),
			ImVec2(16, 16),
			ImVec2(0, 0),
			ImVec2(1, 1),
			4.f))
		{
			ImGui::OpenPopup("SearchResourcePopup");
			myShouldFocusOnAddComponentInput = true;
			mySelectedResourceIndex = -1;
		}

		std::filesystem::path choosenPath;
		if (DrawSearchResourcePopup(aExtensionsToSearchFor, choosenPath))
		{
			aValue = GResourceManager->CreateRef<T>(choosenPath.string(), std::forward<Args&>(aArgs)...);
			aValue->Load();

			result = true;
		}

		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("Search for resource of this type");
		}

		ImGui::PopID();

		return result;
	}
}
