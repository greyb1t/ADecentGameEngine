#pragma once

#include "Engine/ResourceManagement/ResourceRef.h"

class GameObject;

namespace Engine
{
	class Scene;

	class GameObjectPrefab
	{
	public:
		static bool Serialize(GameObject& aGameObject, const Path& aPath);
		bool Serialize(const Path& aPath);
		bool Deserialize(const Path& aPath, const bool aLoadResourceNow = false);
		bool Deserialize2(const nlohmann::json& aGameObjectJson);

		nlohmann::json ToJson() const;

		GameObject& Instantiate(Scene& aScene);

		// Returns the instantited prefab instance
		static GameObject& InstantiatePrefabInstance(
			const GameObjectPrefabRef& aPrefab, Scene& aScene);

		const Shared<GameObject>& GetGameObject() const;

		Shared<GameObject> CreateGameObjectChild();
		void RemoveGameObjectChild(const Shared<GameObject>& aGameObject);

	private:
		int GetNextPrefabGameObjectID();

		static void SerializeGameObject(
			nlohmann::json& aArrayOut,
			GameObject& aGameObject);

		Shared<GameObject> DeserializeGameObject(
			const nlohmann::json& aGameObjectJson,
			const Shared<GameObject>& aParent);

		bool AreAllPrefabGameObjectIDUnique() const;
		static bool IsPrefabIDCounterValid(const int aIDCounterValue, GameObject& aTopPrefabGameObject);

	private:
		friend class InspectableGameObjectPrefab;

		// owned by this class, only shared because it needs to be to conform with existing functions
		Shared<GameObject> myGameObject;

		// Required to keep the children alive and well
		std::vector<Shared<GameObject>> myAllGameObjects;

		int myPrefabGameObjectIDCounter = 0;
		void GenerateNewIDCounter();
	};
}