#include "pch.h"
#include "ImguiReflector.h"

#include "Engine/Engine.h"
#include "Engine/Shortcuts.h"
#include "Engine/Animation/AnimationStateMachine.h"
#include "Engine/Editor/Editor.h"
#include "Engine/Editor/ImGuiHelper.h"
#include "Engine/GameObject/Transform.h"
#include "Engine/Renderer/GraphicsEngine.h"
#include "Engine/Renderer/Material/MaterialFactory.h"
#include "Engine/Renderer/Material/MeshMaterial.h"
#include "Engine/ResourceManagement/Resources/AnimationStateMachineResource.h"
#include "Engine/ResourceManagement/Resources/ModelResource.h"
#include "Engine/ResourceManagement/Resources/MaterialResource.h"
#include "Engine/ResourceManagement/Resources/TextureResource.h"
#include "Engine/ResourceManagement/Resources/GameObjectPrefabResource.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/Scene/Scene.h"
#include <Engine/ResourceManagement/Resources/VisualScriptResource.h>
#include "Engine/Reflection/Enum.h"
#include "Engine/Scene/FolderScene.h"

#include "Engine/Animation/AnimationPair.h"
#include "Engine/Editor/EditorWindows/AssetBrowser/AssetBrowserWindow.h"
#include "Engine/Editor/EditorWindows/Inspector/InspectorWindow.h"
#include "Engine/Renderer/Texture/Texture2D.h"
#include "Engine/ResourceManagement/Resources/AnimationCurveResource.h"
#include "Engine/ResourceManagement/Resources/GameObjectPrefabResource.h"
#include "Engine/Editor/DragDropConstants.h"
#include "Engine/GraphManager/VisualGraphPair.h"
#include "Engine/Editor/FileTypes.h"
#include "FindValueReflector.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/GameObjectRef.h"
#include "Engine/Editor/EditorWindows/SceneHierarchy/SceneHierarchyWindow.h"
#include "Engine/GameObject/RecursiveGameObjectChildIterator.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"

Engine::ImguiReflector::ImguiReflector(
	Editor& aEditor,
	Reflectable* aCompareAgainst,
	GameObject* aPrefabInstanceGameObject)
	: myEditor(aEditor),
	myCompareAgainst(aCompareAgainst),
	myPrefabInstanceGameObject(aPrefabInstanceGameObject)
{
}

Engine::ReflectorResult Engine::ImguiReflector::ReflectInternal(Reflectable& aValue, const std::string& aName,
	const ReflectionFlags aFlags)
{
	ReflectorResult result = ReflectorResult_None;

	const bool oldDraw = myDraw;

	bool b = false;

	if (myDraw)
	{
		b = ImGui::TreeNodeEx(aName.c_str(),
			ImGuiTreeNodeFlags_FramePadding |
			ImGuiTreeNodeFlags_SpanAvailWidth);
	}

	if (b)
	{
		myDraw = true;
	}
	else
	{
		myDraw = false;
	}

	result = Reflector::ReflectInternal(aValue, aName, aFlags);

	if (b)
	{
		ImGui::TreePop();
	}

	myDraw = oldDraw;

	return result;
}

Engine::ReflectorResult Engine::ImguiReflector::ReflectInternal(
	GameObjectRef& aValue,
	const std::string& aName,
	const ReflectionFlags aFlags)
{
	if (myHideNextItem || !myDraw)
	{
		return ReflectorResult_None;
	}

	DrawName(aName, aValue, aFlags);

	ImGui::PushID(&aValue);

	ReflectorResult result = ReflectorResult_None;

	// Channel is like Z-order, widgets in higher channels are rendered above widgets in lower ones
	ImGui::GetWindowDrawList()->ChannelsSplit(2);
	ImGui::GetWindowDrawList()->ChannelsSetCurrent(1);

	// for some reason we need to offset to more to the right
	// otherwise does not line up with other aligned items
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 4.f);

	std::string gameObjectName = "";

	switch (aValue.myType)
	{
	case GameObjectRef::Type::PrefabHierarchyReference:
		if (myPrefabInstanceGameObject)
		{
			GameObject* topmostGameObject = myPrefabInstanceGameObject;

			Transform* topmostParent = myPrefabInstanceGameObject->GetTransform().GetTopMostParent();

			if (topmostParent)
			{
				topmostGameObject = topmostParent->GetGameObject();
			}

			if (topmostGameObject)
			{
				if (topmostGameObject->GetPrefabGameObjectID() == aValue.myPrefabHierarchyReference.myPrefabGameObjectID)
				{
					gameObjectName = topmostGameObject->GetName() + " (" + std::to_string(aValue.myPrefabHierarchyReference.myPrefabGameObjectID) + ")";
					break;
				}

				for (auto it = GameObjectChildIterator(*topmostGameObject); it != nullptr; it++)
				{
					if (it.DerefAsRaw()->GetPrefabGameObjectID() == aValue.myPrefabHierarchyReference.myPrefabGameObjectID)
					{
						gameObjectName = it.DerefAsRaw()->GetName() + " (" + std::to_string(aValue.myPrefabHierarchyReference.myPrefabGameObjectID) + ")";
						break;
					}
				}
			}
		}
		else
		{
			LOG_ERROR(LogType::Editor) << "PrefabInstanceGameObject is null, why?, tell filip";
		}
		break;
	case GameObjectRef::Type::SceneHierarchyReference:
		if (GameObject* g = myEditor.GetActiveScene()->FindGameObject(aValue.mySceneHierarchyReference.myUUID))
		{
			gameObjectName = g->GetName() + " (" + std::to_string(aValue.mySceneHierarchyReference.myUUID) + ")";
		}
		break;
	default:
		break;
	}

	ImGui::AlignTextToFramePadding();
	if (ImGui::Selectable(
		gameObjectName.c_str(),
		false,
		0,
		ImVec2(ImGui::GetContentRegionAvailWidth() - 4.f, 0)))
	{
		switch (aValue.myType)
		{
		case GameObjectRef::Type::PrefabHierarchyReference:
		{
			if (myPrefabInstanceGameObject)
			{
				Shared<GameObject> gameObjectToSelect;

				switch (myEditor.GetSceneHierarchyWindow().GetState())
				{
				case SceneHierarchyWindow::State::DrawGameObjectPrefabHierarchy:
				{
					const auto& topPrefabGameObject = myEditor.GetSceneHierarchyWindow().GetGameObjectPrefab()->Get().GetGameObject();

					bool found = false;

					for (auto it = GameObjectChildIterator(*topPrefabGameObject); it != nullptr; it++)
					{
						if (it.DerefAsRaw()->GetPrefabGameObjectID() == aValue.myPrefabHierarchyReference.myPrefabGameObjectID)
						{
							gameObjectToSelect = it.DerefAsShared();
							found = true;
							break;
						}
					}

					if (!found)
					{
						LOG_ERROR(LogType::Editor) << "Cannot select the gameobject reference";
					}
				} break;

				case SceneHierarchyWindow::State::DrawSceneHierarchy:
				{
					int uuid = -1;

					for (auto it = GameObjectChildIterator(*myPrefabInstanceGameObject); it != nullptr; it++)
					{
						if (aValue.myPrefabHierarchyReference.myPrefabGameObjectID == it.DerefAsRaw()->GetPrefabGameObjectID())
						{
							uuid = it.DerefAsRaw()->GetUUID();
						}
					}

					if (uuid != -1)
					{
						auto g = myEditor.GetActiveScene()->myGameObjectManager.FindGameObjectShared(uuid);
						gameObjectToSelect = g;
					}
				} break;
				default:
					break;
				}

				if (gameObjectToSelect)
				{
					myEditor.PushCommand(myEditor.CreateCommand_SelectGameObjects({ gameObjectToSelect }));
					myEditor.FinishCommand();
				}
			}
			else
			{
				LOG_ERROR(LogType::Engine) << "Cannot select the gameobject, PrefabInstanceGameObject is null";
			}
		} break;
		case GameObjectRef::Type::SceneHierarchyReference:
			if (auto g = myEditor.GetActiveScene()->myGameObjectManager.FindGameObjectShared(aValue.mySceneHierarchyReference.myUUID))
			{
				myEditor.PushCommand(myEditor.CreateCommand_SelectGameObjects({ g }));
				myEditor.FinishCommand();
			}
			else
			{
				LOG_ERROR(LogType::Editor) << "Unable to find gameobject with UUID: " << aValue.mySceneHierarchyReference.myUUID;
			}
			break;
		default:
			break;
		}
	}

	ImGui::RenderFrame(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255, 0, 0, 0), true, ImGui::GetStyle().FrameRounding);

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
		if (ImGui::MenuItem("Clear"))
		{
			aValue = {};
			result = ReflectorResult_Changed | ReflectorResult_Changing;
		}

		ImGui::EndPopup();
	}

	if (ImGui::IsItemHovered())
	{
		const char* refType = GameObjectRef::TypeToString(aValue.myType);

		switch (aValue.myType)
		{
		case GameObjectRef::Type::PrefabHierarchyReference:
			ImGui::SetTooltip("Type: %s, PrefabGameObjectID: %i, RefPointer: 0x%08x", refType, aValue.myPrefabHierarchyReference.myPrefabGameObjectID, reinterpret_cast<uintptr_t>(aValue.myRef.lock().get()));
			break;
		case GameObjectRef::Type::SceneHierarchyReference:
			ImGui::SetTooltip("Type: %s, UUID: %i, RefPointer: 0x%08x", refType, aValue.mySceneHierarchyReference.myUUID, reinterpret_cast<uintptr_t>(aValue.myRef.lock().get()));
			break;
		default:
			break;
		}

	}

	if (const ImGuiPayload* payload = ImGuiHelper::BeginDragDropTarget(DragDropConstants::GameObjectReference.c_str()))
	{
		auto droppedGameObject = *reinterpret_cast<Shared<GameObject>*>(payload->Data);
		assert(payload->DataSize == sizeof(Shared<GameObject>));

		switch (myEditor.GetSceneHierarchyWindow().GetState())
		{
		case SceneHierarchyWindow::State::DrawSceneHierarchy:
			aValue.myType = GameObjectRef::Type::SceneHierarchyReference;
			aValue.mySceneHierarchyReference.myUUID = droppedGameObject->GetUUID();
			aValue.myRef = droppedGameObject;
			break;
		case SceneHierarchyWindow::State::DrawGameObjectPrefabHierarchy:
			aValue.myType = GameObjectRef::Type::PrefabHierarchyReference;
			aValue.myPrefabHierarchyReference.myPrefabGameObjectID = droppedGameObject->GetPrefabGameObjectID();
			aValue.myRef = droppedGameObject;
			break;
		default:
			break;
		}

		result = ReflectorResult_Changed | ReflectorResult_Changing;
	}

	ImGui::PopID();

	return result;
}

void Engine::ImguiReflector::SetNextItemSpeed(const float aSpeed)
{
	mySpeed = aSpeed;
}

void Engine::ImguiReflector::SetNextItemRange(const float aMin, const float aMax)
{
	myRangeMin = aMin;
	myRangeMax = aMax;
}

void Engine::ImguiReflector::SetNextItemTooltip(const std::string& aText)
{
	myTooltipText = aText;
}

void Engine::ImguiReflector::SetNextItemHidden()
{
	myHideNextItem = true;
}

void Engine::ImguiReflector::Header(const std::string& aHeaderText)
{
	ImGui::Dummy(ImVec2(0.f, 6.f));

	ImGui::PushFont(GraphicsEngine::ourFont16Bold);
	// Aligns the text vertially to be center in the table
	ImGui::AlignTextToFramePadding();
	ImGui::Text("%s", aHeaderText.c_str());
	ImGui::PopFont();
}

void Engine::ImguiReflector::Separator()
{
	if (myDraw)
	{
		ImGui::Dummy(ImVec2(0.f, 2.f));
		ImGui::Separator();
		ImGui::Dummy(ImVec2(0.f, 2.f));
	}
}

bool Engine::ImguiReflector::Button(const std::string& aText)
{
	return ImGui::Button(aText.c_str());
}

void Engine::ImguiReflector::ResetAttributes()
{
	mySpeed = 1.f;

	myRangeMax = 0.f;
	myRangeMin = 0.f;

	myTooltipText.clear();

	myHideNextItem = false;
}

void Engine::ImguiReflector::ApplyToPrefab()
{
	assert(false && "not working yet ugh");

	if (myPrefabInstanceGameObject)
	{
		// Assumes to be a component of the prefab gameobject
		int test = 0;

		GameObjectManager myPlaceholderGameObjectManager;

		auto myUnchangedPrefabGameObject = myPrefabInstanceGameObject->GetPrefabGameObject()->Clone(myPlaceholderGameObjectManager, false);

		const auto& myGameObjectPrefab = myPrefabInstanceGameObject->GetPrefab();

		// Copy the gameobjects to avoid issues looping issues because we remove gameobjects
		// when updating prefab, and removing within a loop is a bad idea
		// TODO: Loop backwards, solved
		const std::vector<Shared<GameObject>> gameObjects =
			myEditor.GetActiveScene()->myGameObjectManager.myGameObjects;

		for (auto& gameObject : gameObjects)
		{
			if (gameObject.get() == myPrefabInstanceGameObject)
			{
				continue;
			}

			if (!gameObject->IsPrefabInstance())
			{
				continue;
			}

			GameObject& prefabInstance = *gameObject;
			const auto& prefab = prefabInstance.GetPrefab();

			if (myGameObjectPrefab != prefab)
			{
				continue;
			}

			assert(prefab && prefab->IsValid());

			// When reflecting on each Component & GameObject we assume
			// they their Reflect() layout is exactly the same 1:1

			gameObject->SyncWithPrefabFull(myUnchangedPrefabGameObject.get(), myPrefabInstanceGameObject);
		}

		/*
		if (myGameObjectPrefab->Get().Serialize(myGameObjectPrefab->GetPath()))
		{
			UpdateExistingPrefabs();
		}
		*/

		/*
		myEditor.GetActiveScene()->myGameObjectManager.UpdatePrefabInstances(
			myUnchangedPrefabGameObject.get(),
			myGameObjectPrefab);
		*/
	}
}

Engine::ReflectorResult Engine::ImguiReflector::ReflectInternal(uint64_t& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myHideNextItem || !myDraw)
	{
		return ReflectorResult_None;
	}

	ImGui::PushID(&aValue);

	ReflectorResult result = ReflectorResult_None;

	DrawName(aName, aValue, aFlags);

	ImGui::SameLine(/*ImGui::GetContentRegionAvailWidth() * Editor::ourAlignPercent*/);

	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());

	if (aFlags & ReflectionFlags_ReadOnly)
	{
		ImGui::AlignTextToFramePadding();
		ImGui::Text(std::to_string(aValue).c_str());
	}
	else
	{
		if (ImGui::DragScalar("", ImGuiDataType_U64, &aValue, mySpeed, &myRangeMin, &myRangeMax, "%I64u", 0))
		{
			result |= ReflectorResult_Changing;
		}
	}

	if (ImGui::IsItemDeactivated())
	{
		result |= ReflectorResult_Changed;
	}

	ImGui::PopID();

	return result;
}

Engine::ReflectorResult Engine::ImguiReflector::ReflectInternal(
	int& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myHideNextItem || !myDraw)
	{
		return ReflectorResult_None;
	}

	ImGui::PushID(&aValue);

	ReflectorResult result = ReflectorResult_None;

	DrawName(aName, aValue, aFlags);

	ImGui::SameLine(/*ImGui::GetContentRegionAvailWidth() * Editor::ourAlignPercent*/);

	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());

	if (aFlags & ReflectionFlags_ReadOnly)
	{
		ImGui::AlignTextToFramePadding();
		ImGui::Text(std::to_string(aValue).c_str());
	}
	else
	{
		if (ImGui::DragInt("", &aValue, mySpeed, myRangeMin, myRangeMax))
		{
			result |= ReflectorResult_Changing;
		}
	}

	if (ImGui::IsItemDeactivated())
	{
		result |= ReflectorResult_Changed;
	}

	ImGui::PopID();

	return result;
}

Engine::ReflectorResult Engine::ImguiReflector::ReflectInternal(
	float& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myHideNextItem || !myDraw)
	{
		return ReflectorResult_None;
	}

	ImGui::PushID(&aValue);

	ReflectorResult result = ReflectorResult_None;

	DrawName(aName, aValue, aFlags);

	ImGui::SameLine(/*ImGui::GetContentRegionAvailWidth() * Editor::ourAlignPercent*/);

	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());

	if (aFlags & ReflectionFlags_ReadOnly)
	{
		ImGui::AlignTextToFramePadding();
		ImGui::Text(StringUtilities::FloatToString(aValue, 3).c_str());
	}
	else
	{
		if (ImGui::DragFloat("", &aValue, mySpeed, myRangeMin, myRangeMax))
		{
			result |= ReflectorResult_Changing;
		}
	}

	if (ImGui::IsItemHovered() || ImGui::IsItemActive())
	{
		result |= ReflectorResult_HoveringOverItem;
	}

	if (ImGui::IsItemDeactivated())
	{
		result |= ReflectorResult_Changed;
	}

	ImGui::PopID();

	return result;
}

Engine::ReflectorResult Engine::ImguiReflector::ReflectInternal(
	Vec2f& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myHideNextItem || !myDraw)
	{
		return ReflectorResult_None;
	}

	ImGui::PushID(&aValue);

	ReflectorResult result = ReflectorResult_None;

	DrawName(aName, aValue, aFlags);

	if (ImGui::DragFloat2("", &aValue.x, mySpeed, myRangeMin, myRangeMax))
	{
		result |= ReflectorResult_Changing;
	}

	if (ImGui::IsItemDeactivated())
	{
		result |= ReflectorResult_Changed;
	}

	ImGui::PopID();

	return result;
}

Engine::ReflectorResult Engine::ImguiReflector::ReflectInternal(
	Vec3f& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myHideNextItem || !myDraw)
	{
		return ReflectorResult_None;
	}

	ImGui::PushID(&aValue);

	ReflectorResult result = ReflectorResult_None;

	DrawName(aName, aValue, aFlags);

	if (aFlags & ReflectionFlags_IsColor)
	{
		std::string name = "##" + aName;
		if (ImGui::ColorEdit3(name.c_str(), &aValue.x))
		{
			result |= ReflectorResult_Changing;
		}
	}
	else
	{
		std::string name = "##" + aName;
		if (ImGui::DragFloat3(name.c_str(), &aValue.x, mySpeed, myRangeMin, myRangeMax))
		{
			result |= ReflectorResult_Changing;
		}
	}

	if (auto payLoad = ImGuiHelper::BeginDragDropTarget("Vec3f"))
	{
		assert(payLoad->DataSize == sizeof(Vec3f));
		auto data = reinterpret_cast<Vec3f*>(payLoad->Data);
		aValue = *data;
		result |= ReflectorResult_Changed;
	}

	if (ImGui::IsItemDeactivated())
	{
		result |= ReflectorResult_Changed;
	}

	ImGui::PopID();

	return result;
}

Engine::ReflectorResult Engine::ImguiReflector::ReflectInternal(
	Vec4f& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myHideNextItem || !myDraw)
	{
		return ReflectorResult_None;
	}

	ImGui::PushID(&aValue);

	ReflectorResult result = ReflectorResult_None;

	DrawName(aName, aValue, aFlags);

	if (aFlags & ReflectionFlags_IsColor)
	{
		if (ImGui::ColorEdit4("", &aValue.x))
		{
			result |= ReflectorResult_Changing;
		}
	}
	else
	{
		if (ImGui::DragFloat4("", &aValue.x, mySpeed, myRangeMin, myRangeMax))
		{
			result |= ReflectorResult_Changing;
		}
	}

	if (ImGui::IsItemDeactivated())
	{
		result |= ReflectorResult_Changed;
	}

	ImGui::PopID();

	return result;
}

Engine::ReflectorResult Engine::ImguiReflector::ReflectInternal(
	bool& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myHideNextItem || !myDraw)
	{
		return ReflectorResult_None;
	}

	ImGui::PushID(&aValue);

	ReflectorResult result = ReflectorResult_None;

	DrawName(aName, aValue, aFlags);

	if (ImGui::Checkbox("", &aValue))
	{
		result |= ReflectorResult_Changing;
	}

	if (ImGui::IsItemDeactivated())
	{
		result |= ReflectorResult_Changed;
	}

	ImGui::PopID();

	return result;
}

Engine::ReflectorResult Engine::ImguiReflector::ReflectInternal(
	std::string& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myHideNextItem || !myDraw)
	{
		return ReflectorResult_None;
	}

	ImGui::PushID(&aValue);

	ReflectorResult result = ReflectorResult_None;

	if (aFlags & ReflectionFlags_ReadOnly)
	{
		// Aligns the text vertially to be center in the table
		ImGui::AlignTextToFramePadding();
		ImGui::Text("%s", aName.c_str());

		ImGui::SameLine(/*ImGui::GetContentRegionAvailWidth() * Editor::ourAlignPercent*/);

		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());


		ImGui::Text("%s", aValue.c_str());
	}
	else
	{
		DrawName(aName, aValue, aFlags);

		ImGui::SameLine(/*ImGui::GetContentRegionAvailWidth() * Editor::ourAlignPercent*/);

		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());
		char buffer[512] = {};
		strcpy_s(buffer, aValue.c_str());

		if (ImGui::InputText("", buffer, sizeof(buffer)))
		{
			aValue = buffer;
			result |= ReflectorResult_Changing;
		}

		if (ImGui::IsItemDeactivated())
		{
			result |= ReflectorResult_Changed;
		}
	}

	ImGui::PopID();

	return result;
}

Engine::ReflectorResult Engine::ImguiReflector::ReflectInternal(
	ModelRef& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myHideNextItem || !myDraw)
	{
		return ReflectorResult_None;
	}

	DrawName(aName, aValue, aFlags);

	if (myEditor.DrawReflectedResourceValue(aValue, aName, DragDropConstants::Model, FileType::Extension_Model))
	{
		return ReflectorResult_Changed | ReflectorResult_Changing;
	}

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::ImguiReflector::ReflectInternal(MaterialRef& aValue,
	const MaterialType aType,
	const std::string& aName,
	const ReflectionFlags aFlags)
{
	if (myHideNextItem || !myDraw)
	{
		return ReflectorResult_None;
	}

	const auto rightClickContextMenu = [&aValue, this]()
	{
		if (aValue && aValue->IsValid())
		{
			if (ImGui::MenuItem("Edit material"))
			{
				myEditor.ShowMaterialInInspector(aValue->GetPath());
			}
		}
	};

	DrawName(aName, aValue, aFlags);

	const std::string* dragDropType = nullptr;
	const std::string* extensionToSearch = nullptr;

	switch (aType)
	{
	case MaterialType::Mesh:
		dragDropType = &DragDropConstants::MeshMaterial;
		extensionToSearch = &FileType::Extension_MeshMaterial;
		break;
	case MaterialType::Sprite:
		dragDropType = &DragDropConstants::SpriteMaterial;
		extensionToSearch = &FileType::Extension_SpriteMaterial;
		break;
	case MaterialType::Decal:
		dragDropType = &DragDropConstants::DecalMaterial;
		extensionToSearch = &FileType::Extension_DecalMaterial;
		break;
	case MaterialType::Particle:
		dragDropType = &DragDropConstants::ParticleMaterial;
		extensionToSearch = &FileType::Extension_ParticleMaterial;
		break;
	case MaterialType::Unknown:
		break;
	default:
		break;
	}

	if (myEditor.DrawReflectedResourceValue(
		aValue,
		aName,
		*dragDropType,
		*extensionToSearch,
		rightClickContextMenu))
	{
		return ReflectorResult_Changed | ReflectorResult_Changing;
	}

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::ImguiReflector::ReflectInternal(
	AnimationPair& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myHideNextItem || !myDraw)
	{
		return ReflectorResult_None;
	}

	const auto rightClickContextMenu = [&aValue, this]()
	{
		if (aValue.myMachineInstance)
		{
			if (ImGui::MenuItem("Edit instance"))
			{
				myEditor.OpenAnimationEditorOfExistingInstance(
					aValue.myMachineInstance,
					aValue.myAnimationResource->GetPath());
			}
		}

		if (aValue.myAnimationResource && aValue.myAnimationResource->IsValid())
		{
			if (ImGui::MenuItem("Edit file"))
			{
				myEditor.OpenAnimationEditorNewInstanceOfMachine(aValue.myAnimationResource->GetPath());
			}
		}
	};

	DrawName(aName, aValue, aFlags);

	if (myEditor.DrawReflectedResourceValue(
		aValue.myAnimationResource,
		aName,
		DragDropConstants::AnimationStateMachine,
		FileType::Extension_AnimationStateMachine,
		rightClickContextMenu))
	{
		return ReflectorResult_Changed | ReflectorResult_Changing;
	}

	return ReflectorResult_None;
}
Engine::ReflectorResult Engine::ImguiReflector::ReflectInternal(Enum& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myHideNextItem || !myDraw)
	{
		return ReflectorResult_None;
	}

	ImGui::PushID(&aValue);

	ReflectorResult result = ReflectorResult_None;

	DrawName(aName, aValue, aFlags);

	if (ImGui::BeginCombo(std::string("##" + aName).c_str(), aValue.EnumToString(aValue.GetValue()).c_str()))
	{
		for (int i = 0; i < aValue.GetCount(); i++)
		{
			bool selected = aValue.GetValue() == i;
			if (ImGui::Selectable(aValue.EnumToString(i).c_str(), selected))
			{
				aValue.SetValue(i);
				result |= ReflectorResult_Changing;
				result |= ReflectorResult_Changed;
			}
		}
		ImGui::EndCombo();
	}

	ImGui::PopID();

	return result;
}
void Engine::ImguiReflector::ReflectLambda(const std::function<void(void)>& aLambda)
{
	aLambda();
}
Engine::ReflectorResult Engine::ImguiReflector::ReflectInternal(
	TextureRef& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myHideNextItem || !myDraw)
	{
		return ReflectorResult_None;
	}

	DrawName(aName, aValue, aFlags);

	if (myEditor.DrawReflectedResourceValue(
		aValue,
		aName,
		{ DragDropConstants::Texture, DragDropConstants::HDRTexture },
		{ FileType::Extension_DDS, FileType::Extension_HDR }))
	{
		return ReflectorResult_Changed | ReflectorResult_Changing;
	}

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::ImguiReflector::ReflectInternal(
	VisualGraphPair& aValue,
	const std::string& aName,
	const ReflectionFlags aFlags)
{
	if (myHideNextItem || !myDraw)
	{
		return ReflectorResult_None;
	}

	const auto rightClickContextMenu = [&aValue, this]()
	{
		if (aValue.myGraphInstance)
		{
			if (ImGui::MenuItem("Edit instance"))
			{
				myEditor.OpenVisualGraphNodeEditorOfExistingInstanec(
					aValue.myGraphInstance,
					aValue.myVisualGraphResource->GetPath());
			}
		}

		if (aValue.myVisualGraphResource && aValue.myVisualGraphResource->IsValid())
		{
			if (ImGui::MenuItem("Edit file"))
			{
				myEditor.OpenVisualGraphNodeEditorNewInstance(aValue.myVisualGraphResource->GetPath());
			}
		}
	};

	DrawName(aName, aValue, aFlags);

	if (myEditor.DrawReflectedResourceValue(
		aValue.myVisualGraphResource,
		aName,
		DragDropConstants::VisualScript,
		FileType::Extension_VisualScript,
		rightClickContextMenu))
	{
		return ReflectorResult_Changed | ReflectorResult_Changing;
	}

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::ImguiReflector::ReflectInternal(AnimationCurveRef& aValue,
	const std::string& aName, const ReflectionFlags aFlags)
{
	if (myHideNextItem || !myDraw)
	{
		return ReflectorResult_None;
	}

	DrawName(aName, aValue, aFlags);

	if (myEditor.DrawReflectedResourceValue(aValue, aName, DragDropConstants::AnimationCurve, FileType::Extension_AnimationCurve))
	{
		return ReflectorResult_Changed | ReflectorResult_Changing;
	}

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::ImguiReflector::ReflectInternal(
	GameObjectPrefabRef& aValue,
	const std::string& aName,
	const ReflectionFlags aFlags)
{
	if (myHideNextItem || !myDraw)
	{
		return ReflectorResult_None;
	}

	DrawName(aName, aValue, aFlags);

	if (myEditor.DrawReflectedResourceValue(aValue, aName, DragDropConstants::GameObjectPrefab, FileType::Extension_GameObjectPrefab))
	{
		return ReflectorResult_Changed | ReflectorResult_Changing;
	}

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::ImguiReflector::ReflectInternal(
	VFXRef& aValue,
	const std::string& aName,
	const ReflectionFlags aFlags)
{
	if (myHideNextItem || !myDraw)
	{
		return ReflectorResult_None;
	}

	DrawName(aName, aValue, aFlags);

	if (myEditor.DrawReflectedResourceValue(aValue, aName, DragDropConstants::VFX, FileType::Extension_VFX))
	{
		return ReflectorResult_Changed | ReflectorResult_Changing;
	}

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::ImguiReflector::ReflectInternal(
	Transform& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myHideNextItem || !myDraw)
	{
		return ReflectorResult_None;
	}

	ReflectorResult result = ReflectorResult_None;

	ImGuiHelper::AlignedWidget2("Position", Editor::ourAlignPercent);

	Vec3f localPos = aValue.GetPositionLocal();
	if (ImGui::DragFloat3("##Position", &localPos.x))
	{
		aValue.SetPositionLocal(localPos);
	}

	if (auto payLoad = ImGuiHelper::BeginDragDropTarget("Vec3f"))
	{
		assert(payLoad->DataSize == sizeof(Vec3f));
		auto data = reinterpret_cast<Vec3f*>(payLoad->Data);
		auto pos = aValue.GetPositionLocal();
		pos = *data;
		aValue.SetPositionLocal(pos);
		result |= ReflectorResult_Changed;
	}

	const Quatf localRotation = aValue.GetRotationLocal();

	const Vec3f localRotationRadians = localRotation.EulerAngles();
	Vec3f localRotationDegrees(localRotationRadians.x * Math::RadToDeg,
		localRotationRadians.y * Math::RadToDeg,
		localRotationRadians.z * Math::RadToDeg);

	ImGuiHelper::AlignedWidget2("Rotation", Editor::ourAlignPercent);

	if (ImGui::DragFloat3("##Rotation", &localRotationDegrees.x))
	{
		const Vec3f localRotationRadians(localRotationDegrees.x * Math::DegToRad,
			localRotationDegrees.y * Math::DegToRad,
			localRotationDegrees.z * Math::DegToRad);

		const Quatf newLocalRotation(localRotationRadians);

		aValue.SetRotationLocal(newLocalRotation);
	}

	ImGuiHelper::AlignedWidget2("Scale", Editor::ourAlignPercent);

	Vec3f localScale = aValue.GetScaleLocal();
	if (ImGui::DragFloat3("##Scale", &localScale.x, 0.01f))
	{
		aValue.SetScaleLocal(localScale);
	}

	return result;
}