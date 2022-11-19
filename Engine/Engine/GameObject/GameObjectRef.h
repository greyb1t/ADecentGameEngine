#pragma once

class GameObject;

namespace Engine
{
	class GameObjectPrefabReferencesReflector;
	class ImguiReflector;
	class ToBinaryReflector;
	class FromBinaryReflector;
	class JsonSerializerReflector;
	class JsonDeserializerReflector;

	class GameObjectRef
	{
	public:
		enum class Type
		{
			// A reference to a gameobject in the scene
			SceneHierarchyReference,

			// A reference to a gameobject in its own prefab
			PrefabHierarchyReference,

			Unknown
		};

		GameObjectRef();

		bool IsValid() const;
		GameObject* Get();
		const GameObject* Get() const;

		bool operator==(const GameObjectRef& aOther) const;
		bool operator!=(const GameObjectRef& aOther) const;

		operator bool() const;

		static const char* TypeToString(const Type aType);

	private:
		friend class GameObjectPrefabReferencesReflector;
		friend class ImguiReflector;
		friend class FromBinaryReflector;
		friend class ToBinaryReflector;
		friend class JsonDeserializerReflector;
		friend class JsonSerializerReflector;

		std::weak_ptr<GameObject> myRef;

		union
		{
			struct
			{
				int myPrefabGameObjectID = -1;
			} myPrefabHierarchyReference;

			struct
			{
				int myUUID = -1;
			} mySceneHierarchyReference;
		};

		Type myType = Type::Unknown;
	};
}