#pragma once

#include "SceneSerializer.h"

class GameObject;

namespace Engine
{
	class GameObjectPrefab;

	class SceneSerializerJson : public SceneSerializer
	{
	public:
		virtual ~SceneSerializerJson();

		void Serialize(FolderScene& aFolderScene, const Path& aPath) override;
		bool Deserialize(FolderScene& aFolderScene, const Path& aPath) override;

	private:
		void SerializeGameObjects(
			nlohmann::json& aArrayOut,
			const std::vector<Shared<GameObject>>& aGameObjects);

		void SerializeGameObject(
			nlohmann::json& aArrayOut,
			GameObject& aGameObject);

		void DeserializeGameObject(
			const nlohmann::json& aGameObjectJson,
			FolderScene& aFolderScene,
			GameObject* aParent);

	private:
		// Deserialize variables
		// std::string is the prefab resource Path
		std::unordered_map<std::string, Shared<GameObjectPrefab>> mySavedPrefabGameObjects;
	};
}
