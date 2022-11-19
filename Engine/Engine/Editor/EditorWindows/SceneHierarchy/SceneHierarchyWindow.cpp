#include "pch.h"
#include "SceneHierarchyWindow.h"
#include "Engine/Editor/Editor.h"
#include "Engine/Scene\FolderScene.h"
#include "Engine/GameObject\GameObject.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"
#include "Engine/Editor/ImGuiHelper.h"
#include "Engine/GameObject/Components/MeshComponent.h"
#include "Engine/Renderer/Model/Model.h"
#include "Engine/ResourceManagement/ResourceManager.h"
#include "Engine/ResourceManagement/Resources/ModelResource.h"
#include "Engine\Editor\DragDropConstants.h"
#include "Engine\Editor\Command\Commands\RemoveGameObjectCommand.h"
#include "Engine\Editor\EditorWindows\Inspector\InspectorWindow.h"
#include "Engine\ResourceManagement\Resources\GameObjectPrefabResource.h"
#include "Engine\Editor\Colors.h"
#include "Engine\Editor\EditorWindows\Inspector\Inspectable.h"
#include "imgui_internal.h"
#include "Engine/GameObject/RecursiveGameObjectChildIterator.h"
#include "Engine/GameObject/Components/EditorCameraMovement.h"

#include "Engine/ResourceManagement/ResourceRef.h"
#include "Engine/Editor/EditorWindows/AssetBrowser/AssetBrowserWindow.h"
#include "Engine/ResourceManagement/Resources/TextureResource.h"
#include "Engine/Renderer/Texture/Texture2D.h"

Engine::SceneHierarchyWindow::SceneHierarchyWindow(Editor& aEditor)
	: EditorWindow(aEditor)
{
}

Engine::SceneHierarchyWindow::~SceneHierarchyWindow()
{
}

void Engine::SceneHierarchyWindow::ViewGameObjectPrefab(const GameObjectPrefabRef& aGameObjectPrefab)
{
	myState = State::DrawGameObjectPrefabHierarchy;
	myGameObjectPrefab = aGameObjectPrefab;

	// Set all nodes as open by default
	if (myGameObjectPrefab && myGameObjectPrefab->IsValid())
	{
		myNodeStates[myGameObjectPrefab->Get().GetGameObject()->GetUUID()].myIsOpen = true;

		for (auto it = GameObjectChildIterator(*myGameObjectPrefab->Get().GetGameObject()); it != nullptr; it++)
		{
			myNodeStates[it.DerefAsRaw()->GetUUID()].myIsOpen = true;
		}
	}
}

void Engine::SceneHierarchyWindow::Draw(const float aDeltaTime)
{
	ZoneNamedN(zone1, "SceneHierarchyWindow::Draw", true);

	myOpenedRightClickGameObjectPopup = false;

	for (const auto& [targetIndex, gameObject] : myGameObjectsToMove)
	{
		myEditor.GetActiveScene()->myGameObjectManager.MoveGameObjectToIndex(gameObject, targetIndex);
		//myEditor.PushCommand(myEditor.CreateCommand_MoveGameObject(gameObject, targetIndex));
		//myEditor.FinishCommand();
	}

	myGameObjectsToMove.clear();

	/*
	for (int i = static_cast<int>(myGameObjectsToMove.size()) - 1; i >= 0; --i)
	{
		myEditor.GetActiveScene()->myGameObjectManager.MoveGameObjectToIndex(myGameObjectsToMove[i].first, myGameObjectsToMove[i].second);
	}
	*/

	switch (myState)
	{
	case SceneHierarchyWindow::State::DrawSceneHierarchy:
		DrawSceneHierarchyMain();
		break;
	case SceneHierarchyWindow::State::DrawGameObjectPrefabHierarchy:
		DrawDrawGameObjectPrefabHierarchy();
		break;
	case State::AddResourceRefences:
		DrawAddResourceReferences();
		break;
	default:
		break;
	}
}

void Engine::SceneHierarchyWindow::UpdateFocused(const float aDeltaTime)
{
	if (myState != State::DrawSceneHierarchy)
	{
		return;
	}

	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F), false))
	{
		myEditor.MoveEditorCamToSelection();
	}

	if (myEditor.GetActiveScene2().GetScene())
	{
		if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow), true))
		{
			SelectNextGameObjectAbove();
		}

		if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow), true))
		{
			SelectNextGameObjectBelow();
		}

		if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow), true))
		{
			auto& selection = myEditor.GetActiveScene2().GetSelection();

			if (selection.GetCount() > 0)
			{
				GameObject* gameObject = selection.GetAt(0);

				const bool open = myNodeStates[gameObject->GetUUID()].myIsOpen;

				if (open)
				{
					SelectNextGameObjectBelow();
				}
				else
				{
					myNodeStates[gameObject->GetUUID()].myIsOpen = true;
				}
			}
		}

		if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow), true))
		{
			auto& selection = myEditor.GetActiveScene2().GetSelection();

			if (selection.GetCount() > 0)
			{
				GameObject* gameObject = selection.GetAt(0);

				const bool open = myNodeStates[gameObject->GetUUID()].myIsOpen;

				if (!open)
				{
					// Select node above
					SelectNextGameObjectAbove();
				}
				else
				{
					myNodeStates[gameObject->GetUUID()].myIsOpen = false;
				}
			}
		}
	}

	if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_LeftControl)))
	{
		// Copy
		if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_C)))
		{
			CopySelection();
		}

		// Paste
		if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_V)))
		{
			PasteCopied();
		}
	}

	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Delete)))
	{
		DeleteSelection();
	}
}

Engine::SceneHierarchyWindow::State Engine::SceneHierarchyWindow::GetState() const
{
	return myState;
}

const GameObjectPrefabRef& Engine::SceneHierarchyWindow::GetGameObjectPrefab()
{
	return myGameObjectPrefab;
}

void Engine::SceneHierarchyWindow::DrawSceneGraphCombo()
{
	if (ImGui::BeginCombo(
		"Scene", ourSceneGraphTypeStrings[static_cast<int>(mySelectedGraphType)].c_str()))
	{
		for (int i = 0; i < std::size(ourSceneGraphTypeStrings); ++i)
		{
			if (ImGui::Selectable(ourSceneGraphTypeStrings[i].c_str(),
				i == static_cast<int>(mySelectedGraphType)))
			{
				mySelectedGraphType = static_cast<SceneGraphType>(i);
			}
		}

		ImGui::EndCombo();
	}
}

void Engine::SceneHierarchyWindow::DrawSceneHierarchy()
{
	FolderScene* scene = myEditor.GetActiveScene();

	if (scene == nullptr)
	{
		return;
	}

	switch (mySelectedGraphType)
	{
	case SceneHierarchyWindow::SceneGraphType::OurOwn:
	{
		const auto& gameobjects = scene->myGameObjectManager.GetGameObjects();

		for (size_t i = 0; i < gameobjects.size(); ++i)
		{
			auto& gameobject = gameobjects[i];

			if (gameobject->IsFromUnreal())
			{
				continue;
			}

			// We only draw the top level gameobjects here,
			// the children will be drawn recursively
			if (gameobject->GetTransform().GetParent() == nullptr)
			{
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(myGameObjectSpacing, myGameObjectSpacing));
				ImRect unused;
				DrawGameObjectBranch(gameobject, unused);
				ImGui::PopStyleVar();
			}
		}
	}
	break;

	case SceneHierarchyWindow::SceneGraphType::Unreal:
	{
		const auto& gameobjects = scene->myGameObjectManager.GetGameObjects();

		for (size_t i = 0; i < gameobjects.size(); ++i)
		{
			auto& gameobject = gameobjects[i];

			if (!gameobject->IsFromUnreal())
			{
				continue;
			}

			// We only draw the top level gameobjects here,
			// the children will be drawn recursively
			if (gameobject->GetTransform().GetParent() == nullptr)
			{
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(myGameObjectSpacing, myGameObjectSpacing));
				ImRect unused;
				DrawGameObjectBranch(gameobject, unused);
				ImGui::PopStyleVar();
			}
		}
	}
	break;

	default:
		assert(false);
		break;
	}
}

void Engine::SceneHierarchyWindow::DrawSceneHierarchyMain()
{
	if (ImGui::BeginChild("##SceneGraphDropField", ImGui::GetContentRegionAvail(), false))
	{
		DrawSceneGraphCombo();
		// ImGui::SameLine();
		// if (ImGui::Button("Add Resources"))
		// {
		// 	myState = State::AddResourceRefences;
		// }

		if (ImGui::BeginChild("gameobjectschild"))
		{
			DrawSceneHierarchy();
		}
		ImGui::EndChild();
	}
	ImGui::EndChild();

	/*
	if (ImGui::IsWindowHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Right))
	{
		myEditor.PushCommand(myEditor.CreateCommand_SelectGameObjects({ }));
		myEditor.FinishCommand();

		ImGui::OpenPopup("RightClickEmptySceneHierarchy");
	}
	*/

	HandleSceneGraphDragDrop();

	if (ImGui::BeginPopupContextItem("RightClickEmptySceneHierarchy"))
	{
		DrawRightClickContextMenu();
		ImGui::EndPopup();
	}
}

void Engine::SceneHierarchyWindow::DrawDrawGameObjectPrefabHierarchy()
{
	if (ImGui::Button("Back"))
	{
		myState = State::DrawSceneHierarchy;
		myEditor.GetInspectorWindow().InspectObject(nullptr);
	}

	if (!myGameObjectPrefab || !myGameObjectPrefab->IsValid())
	{
		ImGui::Text("The prefab is not valid, wtf?");
		return;
	}

	const auto& prefabGameObject = myGameObjectPrefab->Get().GetGameObject();

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(myGameObjectSpacing, myGameObjectSpacing));
	ImRect unused;
	DrawGameObjectBranch(prefabGameObject, unused);
	ImGui::PopStyleVar();

	// Must add new gameobject to prefab after all the loops
	// otherwise crash if modifying children while iterating
	if (myCreateNewGameObjectInPrefab)
	{
		myCreateNewGameObjectInPrefab = false;

		myGameObjectPrefab->Get().CreateGameObjectChild();
	}
}

void Engine::SceneHierarchyWindow::DrawGameObjectBranch(
	const Shared<GameObject>& aGameobject, ImRect& aNodeRectOut)
{
	auto& nodeState = myNodeStates[aGameobject->GetUUID()];

	// To fix crash when adding children in the iteration
	bool breakOurOfChildrenLoop = false;

	ImGui::PushID(&aGameobject);

	const std::string& name = aGameobject->GetName();
	const auto& children = aGameobject->GetTransform().GetChildren();
	const int childrenCount = children.size();

	// ImGuiTreeNodeFlags_FramePadding makes the treenode more fat
	ImGuiTreeNodeFlags nodeFlags
		= ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_FramePadding;

	// If no more child, make it a leaf node
	if (childrenCount <= 0)
	{
		nodeFlags |= ImGuiTreeNodeFlags_Leaf;
	}
	else
	{
		nodeFlags |= ImGuiTreeNodeFlags_OpenOnArrow;
	}

	if (myEditor.GetActiveScene2().GetSelection().Exists(aGameobject.get()))
	{
		nodeFlags |= ImGuiTreeNodeFlags_Selected;

		if (myScrollToSelectedItem)
		{
			myScrollToSelectedItem = false;
			ImGui::SetScrollHereY();
		}
	}

	const bool isPrefabInstance = aGameobject->IsPrefabInstance();

	if (aGameobject->IsActive())
	{
		if (isPrefabInstance)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, EditorColors::PrefabTextColor);
		}
		else
		{
			// ImGui::PushStyleColor(ImGuiCol_Text, EditorColors::PrefabTextColor);
		}
	}
	else
	{
		if (isPrefabInstance)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, EditorColors::PrefabTextColorDisabled);
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled)));
		}
	}

	ImGui::PushID(aGameobject.get());

	ImGui::SetNextItemOpen(nodeState.myIsOpen);
	const bool nodeOpen = ImGui::TreeNodeEx(name.c_str(), nodeFlags);
	if (ImGui::IsItemToggledOpen())
	{
		nodeState.myIsOpen = nodeOpen;
	}
	ImGui::PopID();

	aNodeRectOut = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());

	if (aGameobject->IsActive())
	{
		if (isPrefabInstance)
		{
			ImGui::PopStyleColor();
		}
		else
		{
			// ImGui::PushStyleColor(ImGuiCol_Text, EditorColors::PrefabTextColor);
		}
	}
	else
	{
		if (isPrefabInstance)
		{
			ImGui::PopStyleColor();
		}
		else
		{
			ImGui::PopStyleColor();
		}
	}

	// We need this to check if it has already started a payload, otherwise it would do other items in the tree
	if (!ImGui::GetDragDropPayload())
	{
		// not allowed to drag and drop prefab instance child
		const bool isPrefabInstanceChild = aGameobject->IsPrefabInstance() && aGameobject->GetTransform().HasParent();

		if (!isPrefabInstanceChild)
		{
			if (ImGui::BeginDragDropSource())
			{
				// Must be given a pointer to data it will copy
				const Shared<GameObject> ptr = aGameobject;

				ImGui::SetDragDropPayload("GameObject", &ptr, sizeof(Shared<GameObject>), 0);

				ImGui::EndDragDropSource();
			}
		}
	}

	if (const ImGuiPayload* payload = ImGuiHelper::BeginDragDropTarget(
		"GameObject",
		ImVec4(95.f / 255.f, 120.f / 255.f, 90.f / 255.f, 0.0f),
		ImVec4(0, 1, 0, 0.3)))
	{
		if (payload)
		{
			auto droppedGameObject = *reinterpret_cast<Shared<GameObject>*>(payload->Data);
			assert(payload->DataSize == sizeof(Shared<GameObject>));

			// not allowed to normal gameobject onto prefab instance
			// not allowed to drop a prefab instance onto another gameobject
			if (aGameobject != droppedGameObject &&
				!droppedGameObject->IsPrefabInstance() &&
				!aGameobject->IsPrefabInstance())
			{
				bool alreadyIsParent = false;

				if (droppedGameObject->GetTransform().GetParent())
				{
					alreadyIsParent =
						droppedGameObject->GetTransform().GetParent()->GetGameObject() == aGameobject.get();
				}

				if (!alreadyIsParent && !aGameobject->IsChildOf(*droppedGameObject))
				{
					droppedGameObject->GetTransform().SetParent(&aGameobject->GetTransform());
				}
			}
		}
	}

	if (ImGui::IsWindowFocused() && ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
	{
		// Show the inspector when selected a gameobject
		// We cannot focus because it prevent the drag and drop of gameobjects from working
		myEditor.GetInspectorWindow().BringToFront(false);

		if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_LeftControl)))
		{
			myEditor.PushCommand(myEditor.CreateCommand_SelectGameObjectsAppend({ aGameobject }));
			myEditor.FinishCommand();
		}
		else if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_LeftShift)))
		{
			LeftShiftSelectGameObjectsInHierarchy(aGameobject);
		}
		else
		{
			myEditor.PushCommand(myEditor.CreateCommand_SelectGameObjects({ aGameobject }));
			myEditor.FinishCommand();
		}
	}

	if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
	{
		if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_LeftControl)))
		{
			myEditor.PushCommand(myEditor.CreateCommand_SelectGameObjectsAppend({ aGameobject }));
			myEditor.FinishCommand();
		}
		else
		{
			myEditor.PushCommand(myEditor.CreateCommand_SelectGameObjects({ aGameobject }));
			myEditor.FinishCommand();
		}
	}

	if (ImGui::BeginPopupContextItem())
	{
		DrawRightClickContextMenu();

		myOpenedRightClickGameObjectPopup = true;

		ImGui::EndPopup();
	}

	DrawReorderHorizonalLine(aGameobject);

	if (nodeOpen)
	{
		// copy pasta from: https://github.com/ocornut/imgui/issues/2920#issuecomment-615173964
		// filip says thank you
		const ImColor TreeLineColor = ImColor(128, 128, 128, 255);
		const float SmallOffsetX = 1.0f;
		ImDrawList* drawList = ImGui::GetWindowDrawList();

		ImVec2 verticalLineStart = ImGui::GetCursorScreenPos();
		verticalLineStart.x += SmallOffsetX;
		ImVec2 verticalLineEnd = verticalLineStart;

		for (const auto& child : children)
		{
			// We can have a transform without a gameobject if it was created in runtime
			if (child->GetGameObject() == nullptr)
			{
				continue;
			}

			//auto childGameObject
			//	= myEditor.GetActiveScene2().GetScene()->myGameObjectManager.ToShared(
			//		child->GetGameObject());

			auto childGameObject = child->GetGameObjectShared();

			ImRect childRect;

			DrawGameObjectBranch(childGameObject, childRect);

			const float HorizontalTreeLineSize = 8.0f; //chosen arbitrarily
			const float midpoint = (childRect.Min.y + childRect.Max.y) / 2.0f;
			drawList->AddLine(ImVec2(verticalLineStart.x, midpoint), ImVec2(verticalLineStart.x + HorizontalTreeLineSize, midpoint), TreeLineColor);
			verticalLineEnd.y = midpoint;
		}

		drawList->AddLine(verticalLineStart, verticalLineEnd, TreeLineColor);

		ImGui::TreePop();
	}

	ImGui::PopID();
}

void CreateGameObjectFromModelInternal(const ModelRef& aModel,
	GameObject* aGameobject,
	const Engine::FbxNode* aNode)
{
	auto g = aGameobject->GetScene()->AddGameObject<GameObject>();
	g->SetName(aNode->myName);

	auto& t = g->GetTransform();

	t.SetParent(&aGameobject->GetTransform());
	t.SetScaleLocal(aNode->myTransform.GetScaleLocal());
	t.SetPositionLocal(aNode->myTransform.GetPositionLocal());
	t.SetRotationLocal(aNode->myTransform.GetRotationLocal());

	//assert(false && "I changed myMeshIndex to myMeshIndices, now node can have multiple meshes, fix");

	if (aNode->myMeshIndices.size() > 1)
	{
		LOG_WARNING(LogType::Engine) << "Multiple meshes in same not not handled currently, todo handle by simply creating another gameobject for that mesh with same transform";
	}

	if (!aNode->myMeshIndices.empty())
	{
		auto meshComponent = g->AddComponent<Engine::MeshComponent>();
		meshComponent->SetModel(aModel->GetPath());
		meshComponent->SetMeshIndex(aNode->myMeshIndices.front());
		meshComponent->SetMaterial("");
	}

	for (const int childIndex : aNode->myChildrenIndices)
	{
		CreateGameObjectFromModelInternal(aModel, g, &aModel->Get().GetNodes()[childIndex]);
	}
}

void Engine::SceneHierarchyWindow::CreateGameObjectFromModel(
	const ModelRef& aModel, GameObject* aGameobject)
{
	CreateGameObjectFromModelInternal(aModel, aGameobject, &aModel->Get().GetNodes()[0]);

	//std::queue<std::pair<const Renderer::Fbx::Node*, GameObject*>> nodeStack;

	//const auto& nodes = aModel->Get().GetNodes();

	//nodeStack.push(std::make_pair(&nodes.front(), aGameobject));

	//while (!nodeStack.empty())
	//{
	//	const auto& [node, parentGameObject] = nodeStack.front();
	//	nodeStack.pop();

	//	auto g = parentGameObject->GetScene()->AddGameObject<GameObject>();
	//	g->SetName(node->myName);

	//	auto& t = g->GetTransform();

	//	t.SetParent(&parentGameObject->GetTransform());
	//	t.SetScaleLocal(node->myTransform.GetScaleLocal());
	//	t.SetPositionLocal(node->myTransform.GetPositionLocal());
	//	t.SetRotationLocal(node->myTransform.GetRotationLocal());

	//	for (const int childIndex : node->myChildrenIndices)
	//	{
	//		nodeStack.push(std::make_pair(&nodes[childIndex], g));
	//	}
	//}
}

void Engine::SceneHierarchyWindow::CopySelection()
{
	myGameObjectsToCopy = myEditor.GetActiveScene2().GetSelection();
}

void Engine::SceneHierarchyWindow::PasteCopied()
{
	std::vector<Shared<GameObject>> pastedGameObjects;

	for (int i = 0; i < myGameObjectsToCopy.GetCount(); ++i)
	{
		auto gameObject = myGameObjectsToCopy.GetAt(i);

		if (!gameObject)
		{
			continue;
		}

		auto g = gameObject->Clone(myEditor.GetActiveScene2().GetScene()->myGameObjectManager, true);

		pastedGameObjects.push_back(g);

		// Clone() adds it into scene directly, we must remove it
		// and add it again using a command to make sure undo/redo works
		RemoveGameObjectCommand removeIt(*myEditor.GetActiveScene2().GetScene(), g);
		removeIt.Do();

		myEditor.PushCommand(myEditor.CreateCommand_AddGameObject(g));
	}

	// Select the pasted gameobjects
	myEditor.PushCommand(myEditor.CreateCommand_SelectGameObjects(pastedGameObjects));

	myEditor.FinishCommand();
}

void Engine::SceneHierarchyWindow::DeleteSelection()
{
	if (myState == State::DrawSceneHierarchy)
	{
		auto& selection = myEditor.GetActiveScene2().GetSelection();
		std::vector<Shared<GameObject>> gameObjectsToRemove;

		for (int i = 0; i < selection.GetCount(); ++i)
		{
			auto gameObject = selection.GetAt(i);
			gameObject->GetTransform().RemoveParent();
			if (!gameObject)
			{
				continue;
			}

			auto sharedGo = myEditor.GetActiveScene2().GetScene()->myGameObjectManager.ToShared(gameObject);
			gameObjectsToRemove.push_back(sharedGo);
		}

		for (auto& gToRemove : gameObjectsToRemove)
		{
			myEditor.PushCommand(myEditor.CreateCommand_RemoveGameObject(gToRemove));
		}

		myEditor.PushCommand(myEditor.CreateCommand_DeselectGameObjects(gameObjectsToRemove));
		myEditor.FinishCommand();
	}
	else if (myState == State::DrawGameObjectPrefabHierarchy)
	{
		if (!myGameObjectPrefab || !myGameObjectPrefab->IsValid())
		{
			LOG_ERROR(LogType::Editor) << "Prefab is not valid, wtf?";
			return;
		}

		// const auto& prefabGameObject = myGameObjectPrefab->Get().GetGameObject();

		auto& selection = myEditor.GetActiveScene2().GetSelection();

		for (int i = 0; i < selection.GetCount(); ++i)
		{
			myGameObjectPrefab->Get().RemoveGameObjectChild(selection.GetAtWeak(i).lock());
		}
	}
}

void Engine::SceneHierarchyWindow::LeftShiftSelectGameObjectsInHierarchy(const Shared<GameObject>& aGameobject)
{
	auto& selection = myEditor.GetActiveScene2().GetSelection();
	auto& gameobjectManager = myEditor.GetActiveScene2().GetScene()->myGameObjectManager;

	if (selection.GetCount() > 0)
	{
		// if we had already stuff selected, select all in between the already
		// selected gameobject(s) and this one

		int alreadySelectedIndexMin = INT_MAX;
		int alreadySelectedIndexMax = -INT_MAX;

		// Get the already gameobject selection index range
		for (int i = 0; i < selection.GetCount(); ++i)
		{
			const int selectedIndex = gameobjectManager.GetGameObjectIndex(*selection.GetAt(i));
			alreadySelectedIndexMax = std::max(alreadySelectedIndexMax, selectedIndex);
			alreadySelectedIndexMin = std::min(alreadySelectedIndexMin, selectedIndex);
		}

		const int currentSelectedIndex = gameobjectManager.GetGameObjectIndex(*aGameobject);

		assert(currentSelectedIndex != -1);

		// Choose the proper min or max in the selection range to include all the already
		// selected gameobjects into the new selection

		// if the gameobject I just selected with SHIFT, if above both the min/max
		// selection range, then we want to select all between the min index and current selected index
		if (currentSelectedIndex >= alreadySelectedIndexMin &&
			currentSelectedIndex >= alreadySelectedIndexMax)
		{
			std::vector<Shared<GameObject>> gameObjectToSelect;

			// Loop through all gameobjects between min index and current to select them
			for (int i = alreadySelectedIndexMin; i <= currentSelectedIndex; ++i)
			{
				auto g = gameobjectManager.GetGameObjectFromIndex(i);

				// We only select the top level gameobjects with left-shift select
				// not the child hierarchy because the gizmos with the children is fucked atm
				if (g->GetTransform().GetParent() == nullptr)
				{
					gameObjectToSelect.push_back(g);
				}
			}

			myEditor.PushCommand(myEditor.CreateCommand_SelectGameObjects(gameObjectToSelect));
			myEditor.FinishCommand();
		}
		else if (currentSelectedIndex <= alreadySelectedIndexMin &&
			currentSelectedIndex <= alreadySelectedIndexMax)
		{
			std::vector<Shared<GameObject>> gameObjectToSelect;

			// Loop through all gameobjects between min index and current to select them
			for (int i = currentSelectedIndex; i <= alreadySelectedIndexMax; ++i)
			{
				auto g = gameobjectManager.GetGameObjectFromIndex(i);

				// We only select the top level gameobjects with left-shift select
				// not the child hierarchy because the gizmos with the children is fucked atm
				if (g->GetTransform().GetParent() == nullptr)
				{
					gameObjectToSelect.push_back(g);
				}
			}
			myEditor.PushCommand(myEditor.CreateCommand_SelectGameObjects(gameObjectToSelect));
			myEditor.FinishCommand();
		}
		else
		{
			// unsure what to do here atm
			// assert(false && "unsure");

			// If we select something after we already made a left shift selection
			// that is within the already selected range
			// then we want to select between the FIRST selected index and current
		}
	}
	else
	{
		// if nothing was selected yet, do a normal selection of the gameobject
		myEditor.PushCommand(myEditor.CreateCommand_SelectGameObjects({ aGameobject }));
		myEditor.FinishCommand();
	}
}

void Engine::SceneHierarchyWindow::DrawReorderHorizonalLine(const Shared<GameObject>& aGameobject)
{
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

	int offset = 0;

	Shared<GameObject> current = aGameobject;

	//for (Shared<GameObject> current = aGameobject;
	//	current != nullptr && current->GetTransform().GetParent() != nullptr;
	//	current = current->GetTransform().GetParent()->GetGameObjectShared())
	//{

	float originalCursorX = ImGui::GetCursorPosX();

	do
	{
		ImGui::PushID(current.get());
		auto x = ImGui::GetCursorPosX();
		ImGui::InvisibleButton("drag_target_above", ImVec2(ImGui::GetContentRegionAvailWidth(), 4));
		ImGui::SetCursorPosX(x + offset);

		if (const ImGuiPayload* payload = ImGuiHelper::BeginDragDropTarget(
			"GameObject",
			ImVec4(138.f / 255.f, 132.f / 255.f, 0.f, 0.f),
			ImVec4(1.f, 242.f, 0.f, 1.f)))
		{
			// If dragged gameobject below current one
			auto droppedGameObject = *reinterpret_cast<Shared<GameObject>*>(payload->Data);
			assert(payload->DataSize == sizeof(Shared<GameObject>));

			// If current has a parent, means dragged to a gameobject that is not top level and 
			// the dragged gameobject must be added into its hierarchy properly
			const bool draggedToGameObjectChildrenHierarchy = current->GetTransform().GetParent() != nullptr /*|| aGameobject->GetTransform().HasChildren()*/;

			if (draggedToGameObjectChildrenHierarchy &&
				!droppedGameObject->IsPrefabInstance() &&
				!current->IsPrefabInstance())
			{
				// if the dropped gameobject has a parent, means it is also in a child hierarchy
				if (droppedGameObject->GetTransform().GetParent())
				{
					// Remove from its hierarchy
					// Then add into new hierarchy

					const bool draggedOntoItself = droppedGameObject != current;
					if (draggedOntoItself)
					{
						droppedGameObject->GetTransform().SetParent(&current->GetTransform());
					}
				}
				else
				{
					// add into new hierarchy
					droppedGameObject->GetTransform().SetParent(&aGameobject->GetTransform());
				}
			}
			else
			{
				// If dragged to a top level gameobject (no parent)

				if (droppedGameObject->GetTransform().GetParent())
				{
					// if dropped gameobject has parent, remove it
					droppedGameObject->GetTransform().RemoveParent();
				}

				// Move it in the gameobject list
				const int targetIndex =
					myEditor.GetActiveScene()->myGameObjectManager.GetGameObjectIndex(*aGameobject);

				myGameObjectsToMove.push_back(std::make_pair(targetIndex + 1, droppedGameObject));
			}
		}

		offset -= ImGui::GetStyle().IndentSpacing;

		ImGui::PopID();

		if (current->GetTransform().GetParent())
		{
			current = current->GetTransform().GetParent()->GetGameObjectShared();
		}
		else
		{
			current = nullptr;
		}

		// TODO: Fix this
		break;
	} while (current != nullptr);

	ImGui::SetCursorPosX(originalCursorX);

	ImGui::PopStyleVar();
}

void Engine::SceneHierarchyWindow::ScrollToSelectedItem()
{
	myScrollToSelectedItem = true;
}

void Engine::SceneHierarchyWindow::DrawAddResourceReferences()
{
	if (ImGui::Button("Back"))
	{
		myState = State::DrawSceneHierarchy;
	}

	// myEditor.

	//ImGuiHelper::AlignedWidget("Type", Editor::ourAlignPercent);
	//if (ImGui::BeginCombo(
	//	"##resourcetype",
	//	ResourceTypeStrings[static_cast<int>(mySelectedResourceType)].c_str()))
	//{
	//	for (int i = 0; i < static_cast<int>(FileType::FileType::Count); ++i)
	//	{
	//		const auto type = static_cast<FileType::FileType>(i);

	//		if (ImGui::Selectable(
	//			FileType::GlobalFileTypeStrings[i].c_str(),
	//			mySelectedResourceType == type))
	//		{
	//			mySelectedResourceType = type;
	//		}
	//	}

	//	ImGui::EndCombo();
	//}

	//ResourceRef<ResourceBase> resRef;
	//myEditor.DrawReflectedResourceValue(
	//	resRef,
	//	"Resource",
	//	,
	//	".ext");

	// 	for (int i = 0; i < static_cast<int>(FileType::FileType::Count); ++i)
	// 	{
	// 		if (auto payload = ImGuiHelper::BeginDragDropTarget(FileType::GlobalFileTypeStrings[i]))
	// 		{
	// 			char path[255] = {};
	// 			memcpy(path, payload->Data, payload->DataSize);
	// 
	// 			// aValue = GResourceManager->CreateRef<T>(path, std::forward<Args&>(aArgs)...);
	// 			// aValue->Load();
	// 		}
	// 	}

		//switch (mySelectedResourceType)
		//{
		//case ResourceType::AnimationClip:
		//{
		//	AnimationClipRef animClip;
		//	myEditor.DrawReflectedResourceValue(
		//		animClip,
		//		"Animation Clip",
		//		DragDropConstants::AnimationClip,
		//		FileType::Extension_AnimationClip);
		//} break;
		//case ResourceType::AnimationCurve:
		//	break;
		//case ResourceType::AnimationStateMachine:
		//	break;
		//case ResourceType::PixelShader:
		//	break;
		//case ResourceType::VertexShader:
		//	break;
		//case ResourceType::GeometryShader:
		//	break;
		//case ResourceType::MeshMaterial:
		//	break;
		//case ResourceType::Texture:
		//	break;
		//case ResourceType::Model:
		//	break;
		//case ResourceType::VisualScript:
		//	break;
		//case ResourceType::VFX:
		//	break;
		//case ResourceType::GameObjectPrefab:
		//	break;
		//case ResourceType::Count:
		//	break;
		//case ResourceType::Unknown:
		//	break;
		//default:
		//	break;
		//}
}

bool Engine::SceneHierarchyWindow::IsGameObjectNodeVisible(const GameObject& aGameObject) const
{
	for (Transform* parent = aGameObject.GetTransform().GetParent();
		parent != nullptr;
		parent = parent->GetParent())
	{
		auto f = myNodeStates.find(parent->GetGameObject()->GetUUID());

		// If it was not in the node state list its considered not visible
		if (f == myNodeStates.end())
		{
			return false;
		}

		if (!f->second.myIsOpen)
		{
			return false;
		}
	}

	return true;
}

void Engine::SceneHierarchyWindow::SelectNextGameObjectBelow()
{
	// Move selection down
	auto& selection = myEditor.GetActiveScene2().GetSelection();

	if (selection.GetCount() > 0)
	{
		GameObject* gameObject = selection.GetAt(selection.GetCount() - 1);
		int index = myEditor.GetActiveScene()->myGameObjectManager.GetGameObjectIndex(*gameObject);

		Shared<GameObject> gameObjectBelow = myEditor.GetActiveScene()->myGameObjectManager.GetGameObjectFromIndex(std::max(index + 1, 0));

		while (gameObjectBelow && !IsGameObjectNodeVisible(*gameObjectBelow))
		{
			index++;

			gameObjectBelow = myEditor.GetActiveScene()->myGameObjectManager.GetGameObjectFromIndex(std::max(index + 1, 0));
		}

		if (gameObjectBelow)
		{
			myEditor.PushCommand(myEditor.CreateCommand_SelectGameObjects({ gameObjectBelow }));
			myEditor.FinishCommand();
		}
	}
	else
	{
		// if nothing selected, just selected the top item
		if (auto g = myEditor.GetActiveScene()->myGameObjectManager.GetGameObjectFromIndex(0))
		{
			myEditor.PushCommand(myEditor.CreateCommand_SelectGameObjects({ g }));
			myEditor.FinishCommand();
		}
	}

	ScrollToSelectedItem();
}

void Engine::SceneHierarchyWindow::SelectNextGameObjectAbove()
{
	// Move selection up
	auto& selection = myEditor.GetActiveScene2().GetSelection();

	if (selection.GetCount() > 0)
	{
		GameObject* gameObject = selection.GetAt(selection.GetCount() - 1);

		int index = myEditor.GetActiveScene()->myGameObjectManager.GetGameObjectIndex(*gameObject);

		Shared<GameObject> gameObjectAbove = myEditor.GetActiveScene()->myGameObjectManager.GetGameObjectFromIndex(std::max(index - 1, 0));

		while (!IsGameObjectNodeVisible(*gameObjectAbove))
		{
			index--;

			if (index < 0)
			{
				index = 0;
				break;
			}

			gameObjectAbove = myEditor.GetActiveScene()->myGameObjectManager.GetGameObjectFromIndex(std::max(index - 1, 0));
		}

		if (gameObjectAbove)
		{
			myEditor.PushCommand(myEditor.CreateCommand_SelectGameObjects({ gameObjectAbove }));
			myEditor.FinishCommand();
		}
	}
	else
	{
		// if nothing selected, just selected the top item
		if (auto g = myEditor.GetActiveScene()->myGameObjectManager.GetGameObjectFromIndex(0))
		{
			myEditor.PushCommand(myEditor.CreateCommand_SelectGameObjects({ g }));
			myEditor.FinishCommand();
		}
	}

	ScrollToSelectedItem();
}

void Engine::SceneHierarchyWindow::HandleSceneGraphDragDrop()
{
	if (const ImGuiPayload* modelPayload = ImGuiHelper::BeginDragDropTarget(DragDropConstants::Model.c_str()))
	{
		char path[255] = {};
		memcpy(path, modelPayload->Data, modelPayload->DataSize);

		auto model = GResourceManager->CreateRef<ModelResource>(path);
		model->Load();

		Shared<GameObject> g = myEditor.GetActiveScene2().GetScene()->CreateGameObject();

		CreateGameObjectFromModel(model, g.get());

		myEditor.GetActiveScene2().GetScene()->myGameObjectManager.AddGameObject2(g);
	}

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(
			DragDropConstants::GameObjectPrefab.c_str()))
		{
			char path[255] = {};
			memcpy(path, payload->Data, payload->DataSize);

			auto prefabResource = GResourceManager->CreateRef<GameObjectPrefabResource>(path);

			// Ensure we load the associated resources instantly here
			// TODO(filip): Currently I set loadNow in GameObjectPrefab::Deserialize
			// to true to avoid the following issue:
			// When loading a prefab from within another prefab, that inner prefab
			// wont have SetLoadAssociatedResources set to true and won't loadNow
			prefabResource->SetLoadAssociatedResources(true);
			prefabResource->Load();

			// Load the resources associated with the prefab as well


			// TODO: Make it into a resource
			// GameObjectPrefab gameObjectPrefab;

			// if (gameObjectPrefab.Deserialize(path))
			if (prefabResource && prefabResource->IsValid())
			{
				//gameObjectPrefab.Instantiate(*myEditor.GetActiveScene());
				GameObjectPrefab::InstantiatePrefabInstance(prefabResource, *myEditor.GetActiveScene());
			}

			// auto mat = GetEngine().GetResourceManager().CreateRef<FbxResource>(path);
			// mat->Load();
			// aValue = mat;
			//
			// result |= ReflectorResult_Changing | ReflectorResult_Changed;
		}

		ImGui::EndDragDropTarget();
	}
}

void Engine::SceneHierarchyWindow::DrawRightClickContextMenu()
{
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.f, 10.f));

	auto& selection = myEditor.GetActiveScene2().GetSelection();

	if (ImGui::MenuItem("Create new gameobject"))
	{
		if (myState == State::DrawSceneHierarchy)
		{
			Shared<GameObject> g = myEditor.GetActiveScene2().GetScene()->CreateGameObject();
			const auto& editorCamTransform = myEditor.GetEditorCamMovement().GetGameObject()->GetTransform();
			const auto editorCamPos = editorCamTransform.GetPosition();
			const float distanceInfrontOfCam = 400.f;
			g->GetTransform().SetPosition(editorCamPos + editorCamTransform.Forward() * distanceInfrontOfCam);

			myEditor.PushCommand(myEditor.CreateCommand_AddGameObject(g));
			myEditor.PushCommand(myEditor.CreateCommand_SelectGameObjects({ g }));
			myEditor.FinishCommand();

			ScrollToSelectedItem();
		}
		else if (myState == State::DrawGameObjectPrefabHierarchy)
		{
			if (!myGameObjectPrefab || !myGameObjectPrefab->IsValid())
			{
				LOG_ERROR(LogType::Editor) << "Prefab is not valid, wtf?";
				return;
			}

			// const auto& prefabGameObject = myGameObjectPrefab->Get().GetGameObject();

			myCreateNewGameObjectInPrefab = true;
		}
	}

	if (selection.GetCount() > 0)
	{
		if (ImGui::MenuItem("Delete", "DEL"))
		{
			DeleteSelection();
		}

		if (ImGui::MenuItem("Copy", "Ctrl+C"))
		{
			CopySelection();
		}

		if (ImGui::MenuItem("Paste", "Ctrl+V"))
		{
			PasteCopied();
		}
	}

	if (selection.GetCount() == 1)
	{
		GameObject* g = selection.GetAt(0);

		if (g->IsPrefabInstance())
		{
			ImGui::Separator();

			if (ImGui::MenuItem("Open prefab"))
			{
				// Go up in the parents until we find the top level prefab gameobject
				// that has the reference to the actual prefab
				GameObject* currentGameObject = g;

				while (true)
				{
					if (currentGameObject->myPrefab)
					{
						myEditor.ShowGameObjectPrefabInInspector(currentGameObject->myPrefab->GetPath());
						break;
					}

					assert(currentGameObject->GetTransform().GetParent() && "must have parent");

					currentGameObject = currentGameObject->GetTransform().GetParent()->GetGameObject();
				};
			}

			if (ImGui::MenuItem("Unpack prefab"))
			{
				//g->myPrefab = { };

				// Go up in the parents until we find the top level prefab gameobject
				// that has the reference to the actual prefab
				GameObject* currentGameObject = g;

				while (true)
				{
					if (currentGameObject->myPrefab)
					{
						// When found top level prefab gameobject
						// remove the prefab stuff from it and all children

						currentGameObject->myPrefab = {};
						currentGameObject->myPrefabGameObject = nullptr;
						currentGameObject->myPrefabGameObjectID = 0;

						for (auto it = GameObjectChildIterator(*currentGameObject);
							it != nullptr;
							it++)
						{
							it.DerefAsRaw()->myPrefab = {};
							it.DerefAsRaw()->myPrefabGameObject = nullptr;
							it.DerefAsRaw()->myPrefabGameObjectID = 0;
						}

						break;
					}

					assert(currentGameObject->GetTransform().GetParent() && "must have parent");

					currentGameObject = currentGameObject->GetTransform().GetParent()->GetGameObject();
				};
			}

			if (ImGui::MenuItem("Refresh Prefab with Original"))
			{
				// Ta bort prefabben, 

				// Skapa en ny instnans av prefabben, gå igenom varje object på nya och leta efter prefab gameobject ID och återställ gamla transform

				const auto& selectedGameObjectTop = g->GetTransform().GetTopMostParent()->GetGameObjectShared();

				if (selectedGameObjectTop->GetPrefab() && selectedGameObjectTop->GetPrefab()->IsValid())
				{
					const int oldGameObjectIndex = myEditor.GetActiveScene()->myGameObjectManager.GetGameObjectIndex(*selectedGameObjectTop);

					//gameObjectPrefab.Instantiate(*myEditor.GetActiveScene());
					auto& newGameObject = GameObjectPrefab::InstantiatePrefabInstance(selectedGameObjectTop->GetPrefab(), *myEditor.GetActiveScene());

					C::VectorOnStack<GameObject*, 300> oldGameObjectChildren;
					C::VectorOnStack<GameObject*, 300> newGameObjectChildren;

					// Sync the children
					for (auto it = GameObjectChildIterator(*selectedGameObjectTop); it != nullptr; it++)
					{
						oldGameObjectChildren.Add(it.DerefAsRaw());
					}
					oldGameObjectChildren.Add(g);

					for (auto it = GameObjectChildIterator(newGameObject); it != nullptr; it++)
					{
						newGameObjectChildren.Add(it.DerefAsRaw());
					}
					newGameObjectChildren.Add(&newGameObject);

					for (auto& newChild : newGameObjectChildren)
					{
						// Find the matching the gameobjects in the prefab
						const int prefabGameObjectID = newChild->myPrefabGameObjectID;

						auto oldGameObjectFindResult = std::find_if(oldGameObjectChildren.begin(), oldGameObjectChildren.end(),
							[prefabGameObjectID](GameObject* aG1)
							{
								return prefabGameObjectID == aG1->myPrefabGameObjectID;
							});

						// If the gameobject did not exist in any of the
						// prefab gameobjects to compare against, it has
						// been removed, which is handled below in a seperate pass
						if (oldGameObjectFindResult == oldGameObjectChildren.end())
						{
							continue;
						}

						// Copy the transform to the new gameobject from the old
						GameObject* oldChild = *oldGameObjectFindResult;

						const auto& oldTransform = oldChild->GetTransform();

						newChild->GetTransform().SetPositionLocal(oldTransform.GetPositionLocal());
						newChild->GetTransform().SetRotationLocal(oldTransform.GetRotationLocal());
						newChild->GetTransform().SetScaleLocal(oldTransform.GetScaleLocal());
					}

					// Delete old gameobject
					if (myState == State::DrawSceneHierarchy)
					{
						myEditor.PushCommand(myEditor.CreateCommand_RemoveGameObject(selectedGameObjectTop->GetTransform().GetGameObjectShared()));

						myEditor.PushCommand(myEditor.CreateCommand_DeselectGameObjects({ selectedGameObjectTop->GetTransform().GetGameObjectShared() }));
						myEditor.FinishCommand();
					}

					// myEditor.GetActiveScene()->myGameObjectManager.MoveGameObjectToIndex(newGameObject.GetTransform().GetGameObjectShared(), oldGameObjectIndex);

					myGameObjectsToMove.push_back(std::make_pair(oldGameObjectIndex, newGameObject.GetTransform().GetGameObjectShared()));
				}
				else
				{
					LOG_ERROR(LogType::Editor) << "Invalid prefab";
				}

				/*
				if (g->GetPrefab() && g->GetPrefab()->IsValid())
				{
					GameObject* prefabGameObject = g->GetPrefab()->Get().GetGameObject().get();
					g->SyncWithPrefabFull(prefabGameObject, prefabGameObject);
				}
				*/
			}
		}
	}

	ImGui::Separator();

	if (ImGui::MenuItem("Expand Children"))
	{
		GameObject* g = selection.GetAt(0);

		myNodeStates[g->GetUUID()].myIsOpen = true;

		for (auto it = GameObjectChildIterator(*g); it != nullptr; it++)
		{
			myNodeStates[it.DerefAsRaw()->GetUUID()].myIsOpen = true;
		}
	}

	if (ImGui::MenuItem("Collapse Children"))
	{
		GameObject* g = selection.GetAt(0);

		myNodeStates[g->GetUUID()].myIsOpen = false;

		for (auto it = GameObjectChildIterator(*g); it != nullptr; it++)
		{
			myNodeStates[it.DerefAsRaw()->GetUUID()].myIsOpen = false;
		}
	}

	ImGui::PopStyleVar();
}
