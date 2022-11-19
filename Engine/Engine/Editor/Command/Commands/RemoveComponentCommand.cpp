#include "pch.h"
#include "RemoveComponentCommand.h"

#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/Component.h"

Engine::RemoveComponentCommand::RemoveComponentCommand(const Shared<GameObject>& aGameObject, Component* aComponent)
	: myGameObject(aGameObject),
	myComponent(aComponent)
{
}

bool Engine::RemoveComponentCommand::Do()
{
	myGameObject->RemoveComponentNoFree(myComponent);
	return true;
}

bool Engine::RemoveComponentCommand::Undo()
{
	myGameObject->AddExistingComponent(myComponent);
	return true;
}
