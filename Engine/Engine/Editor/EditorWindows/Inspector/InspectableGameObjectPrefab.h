#pragma once

#include "Inspectable.h"
#include "Engine\ResourceManagement\ResourceRef.h"
#include "InspectGameObject.h"
#include "Engine\GameObject\GameObjectManager.h"

namespace Engine
{
	class DecalMaterial;
}

namespace Engine
{
	class InspectableGameObjectPrefab : public Inspectable
	{
	public:
		InspectableGameObjectPrefab(
			Editor& aEditor,
			InspectorWindow& aInspectorWindow,
			const GameObjectPrefabRef& aGameObjectPrefabResource);

		void Draw() override;

		void Save() override;

		std::string GetInspectingType() const override { return "GameObject Prefab"; }

		void SetPrefabGameObject(const Shared<GameObject>& aGameObject);

	private:
		void UpdateExistingPrefabs();

	private:
		GameObjectPrefabRef myGameObjectPrefab;

		static inline float ourAlignPercent = 0.3f;

		InspectGameObject myInspectGameObject;

		Shared<GameObject> myPrefabGameObjectCurrentlyInspected;

		// this GameObjectManager is only used to store the Clone of the prefab gameobject
		// scuffed I know, but I like it
		GameObjectManager myPlaceholderGameObjectManager;
		Shared<GameObject> myUnchangedPrefabGameObject;
	};
}