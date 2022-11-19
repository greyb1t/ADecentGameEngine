#pragma once

#include "Engine/Editor/Command/EditorCommand.h"

class GameObject;
class Component;

namespace Engine
{
	class AddComponentCommand : public EditorCommand
	{
	public:
		AddComponentCommand(const Shared<GameObject>& aGameObject, Component* aComponent);

		bool Do() override;
		bool Undo() override;

	private:
		Shared<GameObject> myGameObject;
		Component* myComponent = nullptr;
	};
}
