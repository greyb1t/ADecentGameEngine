#pragma once

#include "Engine/Editor/Command/EditorCommand.h"

class GameObject;

namespace Engine
{
	class Scene;

	class MoveGameObjectCommand : public EditorCommand
	{
	public:
		MoveGameObjectCommand(
			Scene& aScene,
			const Shared<GameObject>& aGameObjects,
			const int aTargetIndex);

		bool Do() override;
		bool Undo() override;

	private:

	private:
		friend class GameObjectManager;

		Scene& myScene;
		Shared<GameObject> myGameObject;
		int myTargetIndex = -1;
		int myPrevIndex = -1;
	};
}
