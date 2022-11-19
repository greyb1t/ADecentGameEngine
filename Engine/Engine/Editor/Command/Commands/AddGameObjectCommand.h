#pragma once

#include "Engine/Editor/Command/EditorCommand.h"

class GameObject;

namespace Engine
{
	class Scene;

	class AddGameObjectCommand : public EditorCommand
	{
	public:
		AddGameObjectCommand(
			Scene& aScene, 
			const Shared<GameObject>& aGameObjects);

		bool Do() override;
		bool Undo() override;

	private:

	private:
		friend class GameObjectManager;

		Scene& myScene;
		Shared<GameObject> myGameObject;
		std::vector<Shared<GameObject>> myChildren;
	};
}
