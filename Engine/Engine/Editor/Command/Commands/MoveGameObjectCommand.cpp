#include "pch.h"
#include "MoveGameObjectCommand.h"
#include "Engine\Scene\Scene.h"

Engine::MoveGameObjectCommand::MoveGameObjectCommand(Scene& aScene, const Shared<GameObject>& aGameObjects, const int aTargetIndex)
	: myScene(aScene),
	myGameObject(aGameObjects),
	myTargetIndex(aTargetIndex)
{
	myPrevIndex = myScene.myGameObjectManager.GetGameObjectIndex(*myGameObject);
}

bool Engine::MoveGameObjectCommand::Do()
{
	myScene.myGameObjectManager.MoveGameObjectToIndex(myGameObject, myTargetIndex);

	return true;
}

bool Engine::MoveGameObjectCommand::Undo()
{
	myScene.myGameObjectManager.MoveGameObjectToIndex(myGameObject, myPrevIndex);

	return true;
}
