#include "pch.h"
#include "TransformCommand.h"

Engine::TransformCommand::TransformCommand(
	const Shared<GameObject>& aGameObject,
	const Mat4f& aStartTransform,
	const Mat4f& aEndTransform)
	: myGameObject(aGameObject),
	myStartTransform(aStartTransform),
	myEndTransform(aEndTransform)
{
}

bool Engine::TransformCommand::Do()
{
	return true;
}

bool Engine::TransformCommand::Undo()
{
	return true;
}
