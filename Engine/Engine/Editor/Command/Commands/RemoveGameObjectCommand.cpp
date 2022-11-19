#include "pch.h"
#include "RemoveGameObjectCommand.h"

#include "Engine/GameObject/GameObject.h"
#include "Engine/Scene/Scene.h"

Engine::RemoveGameObjectCommand::RemoveGameObjectCommand(
	Scene& aScene, const Shared<GameObject>& aGameObject)
	: myScene(aScene)
	, myGameObject(aGameObject)
{
}

bool Engine::RemoveGameObjectCommand::Do()
{
	myChildren.clear();

	RemoveGameObjectInternal(myGameObject);

	return true;
}

bool Engine::RemoveGameObjectCommand::Undo()
{
	ReAddGameObjectInternal(myGameObject);

	return true;
}

void Engine::RemoveGameObjectCommand::RemoveGameObjectInternal(const Shared<GameObject>& aGameObject)
{
	for (Transform* child : aGameObject->myTransform.GetChildren())
	{
		auto childGo = child->GetGameObjectShared();

		RemoveGameObjectInternal(childGo);
	}

	for (auto& c : aGameObject->myComponents)
	{
		myScene.myGameObjectManager.myComponentSystem.RemoveComponent(c);
	}

	myScene.myGameObjectManager.RemoveGameObject(aGameObject.get());

	myChildren.push_back(aGameObject);
}

void Engine::RemoveGameObjectCommand::ReAddGameObjectInternal(const Shared<GameObject>& aGameObject)
{
	for (Transform* child : aGameObject->myTransform.GetChildren())
	{
		const auto childGo = child->GetGameObjectShared();

		ReAddGameObjectInternal(childGo);
	}

	for (auto& c : aGameObject->myComponents)
	{
		myScene.myGameObjectManager.myComponentSystem.AddExistingComponent(c);
	}

	myScene.myGameObjectManager.AddGameObject2(aGameObject);
}
