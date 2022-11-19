#pragma once

#include "Reflector.h"

class GameObject;

namespace Engine
{
	class GameObjectManager;

	class GameObjectPrefabReferencesReflector : public Reflector
	{
	public:
		GameObjectPrefabReferencesReflector(
			GameObjectManager& aGameObjectManager,
			GameObject& aSourceGameObject);

		ReflectorResult ReflectInternal(
			GameObjectRef& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None) override;

	private:
		GameObjectManager* myGameObjectManager= nullptr;
		GameObject* mySourceGameObject = nullptr;
	};
}
