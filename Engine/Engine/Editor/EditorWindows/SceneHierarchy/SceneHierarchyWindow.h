#pragma once

#include "Engine/Editor/EditorWindows/EditorWindow.h"
#include "Engine/Editor/Selection/Selection.h"
#include "Engine/ResourceManagement/ResourceRef.h"

class GameObject;
struct ImRect;

namespace Engine
{
	class SceneHierarchyWindow : public EditorWindow
	{
	public:
		struct NodeState
		{
			bool myIsOpen = false;
		};

		enum class State
		{
			DrawSceneHierarchy,
			DrawGameObjectPrefabHierarchy,
			AddResourceRefences,
		};

		enum class SceneGraphType
		{
			OurOwn = 0,
			Unreal = 1,

			Count
		};

		const inline static std::string ourSceneGraphTypeStrings[] =
		{
			"Our own",
			"Unreal"
		};

		SceneHierarchyWindow(Editor& aEditor);
		~SceneHierarchyWindow();

		void ViewGameObjectPrefab(const GameObjectPrefabRef& aGameObjectPrefab);

		void Draw(const float aDeltaTime) override;
		void UpdateFocused(const float aDeltaTime) override;

		std::string GetName() const override { return "Scene Graph"; }

		State GetState() const;
		const GameObjectPrefabRef& GetGameObjectPrefab();

	private:
		void DrawSceneHierarchyMain();
		void DrawDrawGameObjectPrefabHierarchy();

		void DrawSceneGraphCombo();
		void DrawSceneHierarchy();

		// Returns true if should continue looping,
		// returns false if should break out of children for loop
		// reason is because we modify the children vector inside
		// the loop when adding new gameobject in gameobject prefab
		void DrawGameObjectBranch(const Shared<GameObject>& aGameobject, ImRect& aNodeRectOut);
		void HandleSceneGraphDragDrop();

		// Returns true if should break our of children iteration loop, edge hack disgusting i know
		void DrawRightClickContextMenu();

		void CreateGameObjectFromModel(const ModelRef& aModel, GameObject* aGameobject);

		void CopySelection();
		void PasteCopied();
		void DeleteSelection();

		void LeftShiftSelectGameObjectsInHierarchy(const Shared<GameObject>& aGameobject);
		void DrawReorderHorizonalLine(const Shared<GameObject>& aGameobject);

		void ScrollToSelectedItem();
		void DrawAddResourceReferences();

		bool IsGameObjectNodeVisible(const GameObject& aGameObject) const;

		void SelectNextGameObjectBelow();
		void SelectNextGameObjectAbove();
	private:
		SceneGraphType mySelectedGraphType = SceneGraphType::OurOwn;

		Selection myGameObjectsToCopy;

		float myGameObjectSpacing = 1.f;

		// <index, GameObject to move>
		std::vector<std::pair<int, Shared<GameObject>>> myGameObjectsToMove;

		State myState = State::DrawSceneHierarchy;

		bool myOpenedRightClickGameObjectPopup = false;

		// only used if myState == DrawGameObjectPrefabHierarchy
		GameObjectPrefabRef myGameObjectPrefab;
		bool myCreateNewGameObjectInPrefab = false;

		bool myScrollToSelectedItem = false;

		// this stores the open/closed treenode state
		std::unordered_map<int, NodeState> myNodeStates;

		// std::vector<std::pair<Shared<GameObject>, int>> myGameObjectsToMove;
	};
}