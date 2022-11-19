#pragma once

#include "Engine/Editor/Command/EditorCommand.h"

class GameObject;
class Component;

namespace Engine
{
	class RemoveComponentCommand : public EditorCommand
	{
	public:
		RemoveComponentCommand(const Shared<GameObject>& aGameObject, Component* aComponent);

		bool Do() override;
		bool Undo() override;

	private:
		Shared<GameObject> myGameObject;
		Component* myComponent = nullptr;
	};
}
