#pragma once

#include "Engine/Editor/Command/EditorCommand.h"

class GameObject;

namespace Engine
{
	class Scene;

	class TransformCommand : public EditorCommand
	{
	public:
		TransformCommand(
			const Shared<GameObject>& aGameObject,
			const Mat4f& aStartTransform,
			const Mat4f& aEndTransform);

		bool Do() override;
		bool Undo() override;

	private:

	private:
		friend class GameObjectManager;

		Shared<GameObject> myGameObject;
		Mat4f myStartTransform;
		Mat4f myEndTransform;
	};
}
