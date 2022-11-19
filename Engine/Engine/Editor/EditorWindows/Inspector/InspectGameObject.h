#pragma once

#include "Engine/ResourceManagement/ResourceRef.h"

class GameObject;
class Component;

namespace Engine
{
	class Editor;

	class InspectGameObject
	{
	public:
		InspectGameObject(
			Editor& aEditor,
			const bool aIsPrefab,
			GameObjectPrefabRef aPrefabResource)
			: myEditor(aEditor),
			myIsPrefab(aIsPrefab),
			myPrefabResource(aPrefabResource)
		{
		}

		void DrawInspectedGameObject(Scene& aScene, const Shared<GameObject>& aGameObject);

	private:
		void DrawAddComponentWindowPopup(Scene& aScene, const Shared<GameObject>& aGameObject);

		void AddComponentsForAllPrefabInstances(
			Scene& aScene,
			GameObject* aPrefabGameObject,
			Component* aComponentToClone);
		void RemoveComponentForAllPrefabInstances(
			Scene& aScene, 
			GameObject* aPrefabGameObject,
			Component* aComponent);

	private:
		bool myShouldFocusOnAddComponentInput = false;
		int mySelectedComponentIndex = 0;

		bool myIsPrefab = false;
		GameObjectPrefabRef myPrefabResource;

		Editor& myEditor;
	};
}