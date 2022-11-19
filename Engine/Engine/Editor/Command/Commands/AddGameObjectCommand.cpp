#include "pch.h"
#include "AddGameObjectCommand.h"

#include "Engine/Scene/Scene.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/RecursiveGameObjectChildIterator.h"

Engine::AddGameObjectCommand::AddGameObjectCommand(
	Scene& aScene, const Shared<GameObject>& aGameObject)
	: myScene(aScene),
	myGameObject(aGameObject)
{
	// Keep the children alive in memory
	for (auto it = GameObjectChildIterator(*aGameObject); it != nullptr; it++)
	{
		myChildren.push_back(it.DerefAsShared());
	}
}

bool Engine::AddGameObjectCommand::Do()
{
	myScene.myGameObjectManager.AddGameObject2(myGameObject);

	for (auto& c : myGameObject->myComponents)
	{
		myScene.myGameObjectManager.myComponentSystem.AddExistingComponent(c);
	}

	for (auto& child : myChildren)
	{
		myScene.myGameObjectManager.AddGameObject2(child);

		for (auto& c : child->myComponents)
		{
			myScene.myGameObjectManager.myComponentSystem.AddExistingComponent(c);
		}
	}

	return true;
}

bool Engine::AddGameObjectCommand::Undo()
{
	myScene.myGameObjectManager.RemoveGameObject(myGameObject.get());

	for (auto& c : myGameObject->myComponents)
	{
		myScene.myGameObjectManager.myComponentSystem.RemoveComponent(c);
	}

	for (auto& child : myChildren)
	{
		myScene.myGameObjectManager.RemoveGameObject(child.get());

		for (auto& c : child->myComponents)
		{
			myScene.myGameObjectManager.myComponentSystem.RemoveComponent(c);
		}
	}

	return true;
}
