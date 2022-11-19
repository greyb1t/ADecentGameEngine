#pragma once

#include "Engine/Editor/Command/EditorCommand.h"

class GameObject;
class Component;

namespace Engine
{
	class Scene;

	class RemoveGameObjectCommand : public EditorCommand
	{
	public:
		RemoveGameObjectCommand(Scene& aScene, const Shared<GameObject>& aGameObject);

		bool Do() override;
		bool Undo() override;

	private:
		void RemoveGameObjectInternal(const Shared<GameObject>& aGameObject);
		void ReAddGameObjectInternal(const Shared<GameObject>& aGameObject);

	private:
		friend class GameObjectManager;

		Scene& myScene;

		Shared<GameObject> myGameObject;

		// Must save children pointers to keep alive in memory
		std::vector<Shared<GameObject>> myChildren;
	};
}
