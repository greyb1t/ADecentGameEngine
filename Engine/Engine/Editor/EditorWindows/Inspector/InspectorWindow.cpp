#include "pch.h"
#include "InspectorWindow.h"
#include "Engine/Editor/Editor.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/Reflection/ImguiReflector.h"
#include "Engine/Scene/FolderScene.h"
#include "Engine/Editor/ImGuiHelper.h"
#include "Inspectable.h"
#include "InspectableSelection.h"
#include "Engine/Editor/FileTypes.h"
#include "Engine/Shortcuts.h"
#include "Engine/ResourceManagement/Resources/GameObjectPrefabResource.h"
#include "InspectableGameObjectPrefab.h"

Engine::InspectorWindow::InspectorWindow(Editor& aEditor)
	: EditorWindow(aEditor, Flags::CanBeUnsaved)
{
}

bool Engine::InspectorWindow::Init(ResourceReferences& aResRefs)
{
	mySearchResourceIcon
		= aResRefs.AddTexture("Assets/Engine/Editor/Icons/SearchResourceIcon.dds");

	return true;
}

void Engine::InspectorWindow::InspectObject(Owned<Inspectable> aInspectable)
{
	myInspectable = std::move(aInspectable);
}

Engine::Inspectable* Engine::InspectorWindow::GetInspectable()
{
	return myInspectable.get();
}

void Engine::InspectorWindow::Draw(const float aDeltaTime)
{
	ZoneNamedN(zone1, "InspectorWindow::Update", true);

	////auto& selection = myEditor.GetActiveScene2().GetSelection();

	////if (selection.GetGameObjects().GetCount() > 0)
	////{
	////	InspectableSelection sel(myEditor, *this, selection);
	////	sel.Draw();
	////}

	////if (selection.GetFilePaths().GetCount() > 0)
	////{
	////	if (selection.GetFilePaths().GetAt(0).has_extension())
	////	{
	////		const auto& filePath = selection.GetFilePaths().GetAt(0);
	////		const FileType::FileType fileType = FileType::ExtensionToFileType(filePath.extension().string());

	////		switch (fileType)
	////		{
	////			case FileType::FileType::PrefabGameObject:
	////			{
	////				auto gameObjectPrefabRef = GResourceManager->CreateRef<GameObjectPrefabResource>(filePath.string());

	////				gameObjectPrefabRef->Load();

	////				if (!gameObjectPrefabRef || !gameObjectPrefabRef->IsValid())
	////				{
	////					LOG_ERROR(LogType::Resource) << "Unable to open " << filePath;
	////					return;
	////				}

	////				//myInspectorWindow->BringToFront();
	////				//myInspectorWindow->InspectObject(MakeOwned<InspectableGameObjectPrefab>(*this, *myInspectorWindow, gameObjectPrefabRef));

	////				InspectableGameObjectPrefab inspectablePrefab(myEditor, *this, gameObjectPrefabRef);
	////				inspectablePrefab.Draw();
	////			} break;
	////			default:
	////				break;
	////		}
	////	}
	////}

	if (myInspectable)
	{
		myInspectable->Draw();
	}

	//auto& selection = myEditor.GetActiveScene2().GetSelection();

	//if (selection.GetCount() == 1)
	//{
	//	auto gameObject = selection.GetAt(0);

	//	if (gameObject)
	//	{
	//		DrawInspectedGameObject(*gameObject);
	//	}
	//}
	//else
	//{
	//	// If multiple gameobjects are selected
	//	// only draw a transform so we can transform them all together
	//}

	//if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootWindow))
	//{
	//	UpdateKeypresses();
	//}
}

TextureRef Engine::InspectorWindow::GetSearchResourceIcon()
{
	return mySearchResourceIcon;
}

//void Engine::InspectorWindow::DrawAddComponentWindowPopup()
//{
//	if (ImGui::BeginPopup("AddComponentPopup"))
//	{
//		static char componentNameBuffer[255] = { };
//
//		const auto addComponent = [&](Component* component)
//		{
//			// add component here
//			if (const auto c = component->Clone())
//			{
//				auto& scene = myEditor.GetActiveScene2();
//				assert(scene.GetSelection().GetCount() > 0);
//
//				GameObject* gameObject = scene.GetSelection().GetAt(0);
//
//				assert(gameObject && "nullptr, sadge :(");
//
//				auto cmd = myEditor.CreateCommand_AddComponent(
//					scene.GetScene()->myGameObjectManager.ToShared(gameObject), c);
//				myEditor.PushCommand(std::move(cmd));
//				myEditor.FinishCommand();
//
//				memset(componentNameBuffer, 0, sizeof(componentNameBuffer));
//			}
//			else
//			{
//				LOG_ERROR(LogType::Editor) << "Could add component, must implement Component::Clone()";
//			}
//			ImGui::CloseCurrentPopup();
//		};
//
//		std::vector<Component*> componentsInList;
//
//		// focus on the text input
//		if (myShouldFocusOnAddComponentInput)
//		{
//			myShouldFocusOnAddComponentInput = false;
//
//			ImGui::SetKeyboardFocusHere();
//		}
//
//		if (ImGui::InputTextWithHint("##", "Search component", componentNameBuffer, sizeof(componentNameBuffer)))
//		{
//			// when we change search, remove the selection
//			mySelectedComponentIndex = -1;
//		}
//
//		componentsInList.clear();
//
//		const std::string componentNameStr = componentNameBuffer;
//
//		if (componentNameStr.empty())
//		{
//			for (int i = 0; i < ComponentFactory::GetRegisteredComponentsCount(); ++i)
//			{
//				auto regComp = ComponentFactory::GetRegisteredComponentFromId(i);
//				if (!regComp)
//				{
//					continue;
//				}
//				Component* c = regComp->myComponent;
//
//				if (c == nullptr)
//				{
//					break;
//				}
//
//				componentsInList.push_back(c);
//			}
//		}
//		else
//		{
//			for (int i = 0; i < ComponentFactory::GetRegisteredComponentsCount(); ++i)
//			{
//				const auto registeredComponent = ComponentFactory::GetRegisteredComponentFromId(i);
//				if (!registeredComponent)
//				{
//					continue;
//				}
//
//				const std::string& componentName = registeredComponent->myName;
//
//				// Lower case string search, good? bad? not sure
//				auto findResult = std::search(
//					componentName.begin(), componentName.end(), componentNameStr.begin(), componentNameStr.end(), [](char c1, char c2)
//					{
//						return toupper(c1) == toupper(c2);
//					});
//
//				if (findResult != componentName.end())
//				{
//					Component* c = registeredComponent->myComponent;
//
//					if (c == nullptr)
//					{
//						break;
//					}
//
//					componentsInList.push_back(c);
//				}
//			}
//		}
//
//		if (ImGui::BeginListBox("##"))
//		{
//			for (int i = 0; i < componentsInList.size(); ++i)
//			{
//				const auto& component = componentsInList[i];
//
//				const auto registeredComponent = ComponentFactory::GetRegisteredComponentFromId(component->GetComponentTypeID());
//
//				const std::string& name = registeredComponent->myName;
//
//				if (ImGui::Selectable(name.c_str(), i == mySelectedComponentIndex))
//				{
//					addComponent(component);
//					break;
//				}
//			}
//
//			ImGui::EndListBox();
//		}
//
//		if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow), false))
//		{
//			--mySelectedComponentIndex;
//			mySelectedComponentIndex = std::max(mySelectedComponentIndex, 0);
//		}
//
//		if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow), false))
//		{
//			++mySelectedComponentIndex;
//			mySelectedComponentIndex = std::min(mySelectedComponentIndex, static_cast<int>(componentsInList.size()) - 1);
//		}
//
//		if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter), false))
//		{
//			if (mySelectedComponentIndex != -1)
//			{
//				addComponent(componentsInList[mySelectedComponentIndex]);
//			}
//		}
//
//		ImGui::EndPopup();
//	}
//}

//void Engine::InspectorWindow::DrawInspectedGameObject(GameObject& aGameObject)
//{
//	ImguiReflector reflector(myEditor);
//
//	if (ImGui::CollapsingHeader("GameObject", ImGuiTreeNodeFlags_DefaultOpen))
//	{
//		aGameObject.Reflect(reflector);
//	}
//
//	ImGui::Dummy(ImVec2(0.f, 30.f));
//
//	// if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
//	// {
//	// 	DrawTransform(aGameObject.GetTransform());
//	// }
//
//	// try to combine serialize and imgui rendering in same code
//	// then no duplicate needs to be done
//	// so if we implement serialize, we can implement drawing of the component
//
//	for (int i = 0; i < aGameObject.myComponents.size(); ++i)
//	{
//		const auto& component = aGameObject.myComponents[i];
//
//		ImGui::PushID(component);
//
//		auto regComponent = ComponentFactory::GetRegisteredComponentFromId(component->GetComponentTypeID());
//
//		auto p = ImGui::GetCursorPos();
//
//		if (regComponent)
//		{
//			ImGui::PushStyleColor(ImGuiCol_Header, ImGuiHelper::CalculateColorFromString(regComponent->myName));
//			const bool isOpen = ImGui::CollapsingHeader(regComponent->myName.c_str(), ImGuiTreeNodeFlags_DefaultOpen);
//			ImGui::PopStyleColor();
//
//			if (ImGui::BeginPopupContextItem())
//			{
//				if (ImGui::Selectable("Remove Component"))
//				{
//					auto cmd = myEditor.CreateCommand_RemoveComponent(
//						myEditor.GetActiveScene2().GetScene()->myGameObjectManager.ToShared(&aGameObject), component);
//					myEditor.PushCommand(std::move(cmd));
//					myEditor.FinishCommand();
//				}
//
//				ImGui::EndPopup();
//			}
//
//			if (isOpen)
//			{
//				component->Reflect(reflector);
//			}
//		}
//		else
//		{
//			ImGui::CollapsingHeader(("Unregistered Component: " + std::string(typeid(component).name())).c_str());
//		}
//
//		// if not last item, add Y spacing
//		if (i < (aGameObject.myComponents.size() - 1))
//		{
//			ImGui::Dummy(ImVec2(0.f, 10.f));
//		}
//
//		ImGui::PopID();
//	}
//
//	if (ImGui::Button("Add Component"))
//	{
//		ImGui::OpenPopup("AddComponentPopup");
//		myShouldFocusOnAddComponentInput = true;
//	}
//
//	DrawAddComponentWindowPopup();
//}

void Engine::InspectorWindow::DrawTransform(Transform& aTransform)
{
	Vec3f localPos = aTransform.GetPositionLocal();
	if (ImGui::DragFloat3("Position", &localPos.x))
	{
		aTransform.SetPositionLocal(localPos);
	}

	const Quatf localRotation = aTransform.GetRotationLocal();

	const Vec3f localRotationRadians = localRotation.EulerAngles();
	Vec3f localRotationDegrees(
		localRotationRadians.x * Math::RadToDeg,
		localRotationRadians.y * Math::RadToDeg,
		localRotationRadians.z * Math::RadToDeg);

	if (ImGui::DragFloat3("Rotation", &localRotationDegrees.x))
	{
		const Vec3f localRotationRadians(
			localRotationDegrees.x * Math::DegToRad,
			localRotationDegrees.y * Math::DegToRad,
			localRotationDegrees.z * Math::DegToRad);

		const Quatf newLocalRotation(localRotationRadians);

		aTransform.SetRotationLocal(newLocalRotation);
	}

	Vec3f localScale = aTransform.GetScaleLocal();
	if (ImGui::DragFloat3("Scale", &localScale.x, 0.01f))
	{
		aTransform.SetScaleLocal(localScale);
	}
}

void Engine::InspectorWindow::UpdateKeypresses()
{
}

void Engine::InspectorWindow::Save()
{
	EditorWindow::Save();

	if (myInspectable)
	{
		myInspectable->Save();
	}
}
