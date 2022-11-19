#include "pch.h"
#include "InspectGameObject.h"
#include "Engine\GameObject\Components\Component.h"
#include "Engine\Reflection\ImguiReflector.h"
#include "Engine\GameObject\GameObject.h"
#include "Engine\Editor\ImGuiHelper.h"
#include "Engine\Editor\Editor.h"
#include "Engine\Scene\FolderScene.h"
#include "Engine\GameObject\Prefab\GameObjectPrefabInstance.h"
#include "Engine\ResourceManagement\Resources\GameObjectPrefabResource.h"
#include "Engine\ResourceManagement\ResourceRef.h"
#include "Engine\Editor\Colors.h"
#include "Engine\GameObject\Prefab\GameObjectPrefab.h"
#include "Engine\GameObject\RecursiveGameObjectChildIterator.h"

void Engine::InspectGameObject::DrawAddComponentWindowPopup(Scene& aScene, const Shared<GameObject>& aGameObject)
{
	if (ImGui::BeginPopup("AddComponentPopup"))
	{
		static char componentNameBuffer[255] = {};

		const auto addComponent = [&](Component* component)
		{
			//if (aGameObject->GetComponentByTypeId(component->GetComponentTypeID()) != nullptr)
			//{
			//	LOG_WARNING(LogType::Editor) << "The component already exists, we cannot have multiple components, det fackar med Prefabs, snacka med Filip om verkligen needed";
			//	return;
			//}

			// add component here
			if (const auto c = component->Clone())
			{
				//auto& scene = myEditor.GetActiveScene2();
				//assert(scene.GetSelection().GetCount() > 0);
				//
				//GameObject* gameObject = scene.GetSelection().GetAt(0);
				//
				//assert(gameObject && "nullptr, sadge :(");

				// NOTE(filip): atm we dont use undo/redo for the prefabs because it would
				// require more works to make work properly because they change all instances in the scene
				if (myIsPrefab)
				{
					aGameObject->AddExistingComponent(c);

					// AddComponentsForAllPrefabInstances(aScene, aGameObject.get(), component);
				}
				else
				{
					auto cmd = myEditor.CreateCommand_AddComponent(
						aGameObject, c);
					myEditor.PushCommand(std::move(cmd));
					myEditor.FinishCommand();
				}

				memset(componentNameBuffer, 0, sizeof(componentNameBuffer));
			}
			else
			{
				LOG_ERROR(LogType::Editor) << "Could add component, must implement Component::Clone()";
			}
			ImGui::CloseCurrentPopup();
		};

		std::vector<Component*> componentsInList;

		// focus on the text input
		if (myShouldFocusOnAddComponentInput)
		{
			myShouldFocusOnAddComponentInput = false;

			ImGui::SetKeyboardFocusHere();
		}

		if (ImGui::InputTextWithHint("##", "Search component", componentNameBuffer, sizeof(componentNameBuffer)))
		{
			mySelectedComponentIndex = 0;
		}

		componentsInList.clear();

		const std::string componentNameStr = componentNameBuffer;

		if (componentNameStr.empty())
		{
			for (int i = 0; i < ComponentFactory::GetInstance().GetRegisteredComponentsCount(); ++i)
			{
				auto regComp = ComponentFactory::GetInstance().GetRegisteredComponentFromId(i);
				if (!regComp)
				{
					continue;
				}
				Component* c = regComp->myComponent;

				if (c == nullptr)
				{
					break;
				}

				componentsInList.push_back(c);
			}
		}
		else
		{
			// Search using keys separated by spaces
			const std::vector<std::string> searchKeys = StringUtilities::Split(componentNameStr, " ");

			for (int i = 0; i < ComponentFactory::GetInstance().GetRegisteredComponentsCount(); ++i)
			{
				const auto registeredComponent = ComponentFactory::GetInstance().GetRegisteredComponentFromId(i);
				if (!registeredComponent)
				{
					continue;
				}

				const std::string& componentName = registeredComponent->myName;

				bool allKeysMatch = true;

				for (const std::string& searchKey : searchKeys)
				{
					// Lower case string search, good? bad? not sure
					auto findResult = std::search(
						componentName.begin(),
						componentName.end(),
						searchKey.begin(),
						searchKey.end(),
						[](char c1, char c2)
						{
							return toupper(c1) == toupper(c2);
						});

					if (findResult == componentName.end())
					{
						allKeysMatch = false;
						break;
					}
				}

				if (allKeysMatch)
				{
					Component* c = registeredComponent->myComponent;

					if (c == nullptr)
					{
						break;
					}

					componentsInList.push_back(c);
				}
			}
		}

		if (ImGui::BeginListBox("##"))
		{
			for (int i = 0; i < componentsInList.size(); ++i)
			{
				const auto& component = componentsInList[i];

				const auto registeredComponent = ComponentFactory::GetInstance().GetRegisteredComponentFromId(component->GetComponentTypeID());

				const std::string& name = registeredComponent->myName;

				if (ImGui::Selectable(name.c_str(), i == mySelectedComponentIndex))
				{
					addComponent(component);
					break;
				}
			}

			ImGui::EndListBox();
		}

		if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow), false))
		{
			--mySelectedComponentIndex;
			mySelectedComponentIndex = std::max(mySelectedComponentIndex, 0);
		}

		if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow), false))
		{
			++mySelectedComponentIndex;
			mySelectedComponentIndex = std::min(mySelectedComponentIndex, static_cast<int>(componentsInList.size()) - 1);
		}

		if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter), false))
		{
			if (mySelectedComponentIndex != -1)
			{
				if (mySelectedComponentIndex < componentsInList.size())
				{
					addComponent(componentsInList[mySelectedComponentIndex]);
				}
			}
		}

		ImGui::EndPopup();
	}
}

void Engine::InspectGameObject::AddComponentsForAllPrefabInstances(
	Scene& aScene,
	GameObject* aPrefabGameObject,
	Component* aComponentToClone)
{
	assert(false && "unused");

	for (auto& gameObject : aScene.GetGameObjects())
	{
		if (!gameObject->IsPrefabInstance())
		{
			continue;
		}

		GameObject& prefabInstance = *gameObject;

		if (prefabInstance.GetPrefabGameObject() != aPrefabGameObject)
		{
			continue;
		}

		assert(prefabInstance.GetPrefabGameObject());

		//const auto& prefab = prefabInstance.GetPrefab();
		//
		//if (myPrefabResource != prefab)
		//{
		//	continue;
		//}
		//
		//assert(prefab && prefab->IsValid());

		gameObject->AddExistingComponent(aComponentToClone->Clone());
	}
}

void Engine::InspectGameObject::RemoveComponentForAllPrefabInstances(
	Scene& aScene,
	GameObject* aPrefabGameObject,
	Component* aComponent)
{
	for (auto& gameObject : aScene.GetGameObjects())
	{
		if (!gameObject->IsPrefabInstance())
		{
			continue;
		}

		GameObject& prefabInstance = *gameObject;

		if (prefabInstance.GetPrefabGameObject() != aPrefabGameObject)
		{
			continue;
		}

		assert(prefabInstance.GetPrefabGameObject());

		//const auto& prefab = prefabInstance.GetPrefab();
		//
		//if (myPrefabResource != prefab)
		//{
		//	continue;
		//}
		//
		//assert(prefab && prefab->IsValid());

		// Remove all components of the same type
		for (auto& c : gameObject->myComponents)
		{
			if (aComponent->GetComponentTypeID() == c->GetComponentTypeID())
			{
				gameObject->RemoveComponentNoFree(c);
			}
		}
	}
}

void Engine::InspectGameObject::DrawInspectedGameObject(Scene& aScene, const Shared<GameObject>& aGameObject)
{
	const bool isPrefabInstance = aGameObject->IsPrefabInstance();

	if (isPrefabInstance)
	{
		// ImGui::PushStyleColor(ImGuiCol_Text, EditorColors::PrefabTextColor);
	}

	Reflectable* compareAgainst = nullptr;

	// If its a prefab, we want to compare against the prefab value in the while reflecting the imgui
	// to be able to show if a value has been overidden
	if (isPrefabInstance)
	{
		compareAgainst = aGameObject->GetPrefabGameObject();
	}

	if (ImGui::CollapsingHeader("GameObject", ImGuiTreeNodeFlags_DefaultOpen))
	{
		const bool isActive = aGameObject->IsActive();

		ImguiReflector reflector(myEditor, compareAgainst, aGameObject.get());
		aGameObject->Reflect(reflector);

		if (isActive != aGameObject->IsActive())
		{
			for (auto it = GameObjectChildIterator(*aGameObject); it != nullptr; it++)
			{
				it.DerefAsRaw()->myIsActive = aGameObject->IsActive();
			}

			if (aGameObject->IsActive())
			{
				// Recursively iterate parents to set them active
				Transform* currentParent = aGameObject->GetTransform().GetParent();

				while (currentParent != nullptr)
				{
					currentParent->GetGameObject()->myIsActive = true;

					currentParent = currentParent->GetParent();
				}
			}
		}
	}

	// ImGui::Dummy(ImVec2(0.f, 30.f));

	for (int i = 0; i < aGameObject->myComponents.size(); ++i)
	{
		const auto& component = aGameObject->myComponents[i];

		ImGui::PushID(component);

		auto regComponent = ComponentFactory::GetInstance().GetRegisteredComponentFromId(component->GetComponentTypeID());

		auto p = ImGui::GetCursorPos();

		std::string nameModifier = "";

		if (isPrefabInstance)
		{
			// if is does not exist in the prefab, means it only exists for this instance
			if (aGameObject->GetPrefabGameObject()->GetComponentByTypeId(component->GetComponentTypeID()) == nullptr)
			{
				nameModifier = " +";
			}
		}

		if (regComponent)
		{
			ImGui::PushStyleColor(ImGuiCol_Header, ImGuiHelper::CalculateColorFromString(regComponent->myName));
			const bool isOpen = ImGui::CollapsingHeader((regComponent->myName + nameModifier).c_str(), ImGuiTreeNodeFlags_DefaultOpen);
			ImGui::PopStyleColor();

			if (ImGui::BeginPopupContextItem())
			{
				if (ImGui::MenuItem("Remove component"))
				{
					if (myIsPrefab)
					{
						aGameObject->RemoveComponentNoFree(component);

						// RemoveComponentForAllPrefabInstances(aScene, aGameObject.get(), component);
					}
					else
					{
						auto cmd = myEditor.CreateCommand_RemoveComponent(
							aGameObject, component);
						myEditor.PushCommand(std::move(cmd));
						myEditor.FinishCommand();
					}
				}

				if (isPrefabInstance)
				{
					if (ImGui::MenuItem("Revert to prefab"))
					{
						// Component* gameObjectPrefabComponent =
						// 	aGameObject->GetPrefabGameObject()->GetComponentByTypeId(component->GetComponentTypeID());
						// 
						// Component* newComponent = gameObjectPrefabComponent->Clone();

						// TODO: Use the GameObjectPrefabReflector to copy the values from the prefab component in the gameobject component, but rename GameObjectPrefabReflector to CopyValuesReflector
					}
				}

				ImGui::EndPopup();
			}

			if (isOpen)
			{
				// If its a prefab, we want to compare against the prefab value in the while reflecting the imgui
				// to be able to show if a value has been overidden
				if (isPrefabInstance)
				{
					compareAgainst = aGameObject->GetPrefabGameObject()->GetComponentByTypeId(component->GetComponentTypeID());
				}

				ImguiReflector reflector(myEditor, compareAgainst, aGameObject.get());
				component->Reflect(reflector);
			}
		}
		else
		{
			// ImGui::CollapsingHeader(("Unregistered Component: " + std::string(typeid(component).name())).c_str());
		}

		// if not last item, add Y spacing
		if (i < (aGameObject->myComponents.size() - 1))
		{
			ImGui::Dummy(ImVec2(0.f, 10.f));
		}

		ImGui::PopID();
	}

	if (isPrefabInstance)
	{
		// Draw all the components that exist in the prefab but not the instance
		for (const Component* c : aGameObject->GetPrefabGameObject()->myComponents)
		{
			if (aGameObject->GetComponentByTypeId(c->GetComponentTypeID()) == nullptr)
			{
				auto regComponent = ComponentFactory::GetInstance().GetRegisteredComponentFromId(c->GetComponentTypeID());

				ImGui::PushStyleColor(ImGuiCol_Header, ImGuiHelper::CalculateColorFromString(regComponent->myName));
				const bool isOpen = ImGui::CollapsingHeader((regComponent->myName + " -").c_str(), ImGuiTreeNodeFlags_DefaultOpen);
				ImGui::PopStyleColor();

				if (ImGui::BeginPopupContextItem())
				{
					if (ImGui::MenuItem("Revert component"))
					{
						// find component in prefab gameobject
						// clone that component
						Component* newComponent = c->Clone();

						// add that component into current gameobject
						aGameObject->AddExistingComponent(newComponent);
					}

					ImGui::EndPopup();
				}
			}
		}
	}

	ImGui::Dummy(ImVec2(0, 4));
	ImGui::Separator();
	ImGui::Dummy(ImVec2(0, 4));

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20);
	if (ImGui::Button("Add Component", ImVec2(-20, 0)))
	{
		ImGui::OpenPopup("AddComponentPopup");
		myShouldFocusOnAddComponentInput = true;
	}

	DrawAddComponentWindowPopup(aScene, aGameObject);

	if (isPrefabInstance)
	{
		//ImGui::PopStyleColor();
	}
}