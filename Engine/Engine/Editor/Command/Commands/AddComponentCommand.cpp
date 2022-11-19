#include "pch.h"
#include "AddComponentCommand.h"

#include "Engine/GameObject/GameObject.h"

Engine::AddComponentCommand::AddComponentCommand(const Shared<GameObject>& aGameObject, Component* aComponent)
	: myGameObject(aGameObject),
	myComponent(aComponent)
{
}

bool Engine::AddComponentCommand::Do()
{
	myGameObject->AddExistingComponent(myComponent);
	return true;
}

bool Engine::AddComponentCommand::Undo()
{
	myGameObject->RemoveComponentNoFree(myComponent);
	return true;
}
