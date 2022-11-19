#include "pch.h"
#include "RecursiveGameObjectChildIterator.h"
#include "Transform.h"
#include "GameObject.h"

Engine::GameObjectChildIterator::GameObjectChildIterator(GameObject& aGameObject)
{
	if (!aGameObject.GetTransform().myChildren.empty())
	{
		for (const auto& child : aGameObject.GetTransform().myChildren)
		{
			myTransformStack.push(child);
		}

		myCurrentTransform = myTransformStack.front();
		myTransformStack.pop();

		for (const auto& child : myCurrentTransform->myChildren)
		{
			myTransformStack.push(child);
		}
	}
	else
	{
		myCurrentTransform = nullptr;
	}
}

Engine::GameObjectChildIterator& Engine::GameObjectChildIterator::operator++(int)
{
	if (myTransformStack.empty())
	{
		myCurrentTransform = nullptr;
		return *this;
	}

	myCurrentTransform = myTransformStack.front();
	myTransformStack.pop();

	for (const auto& child : myCurrentTransform->myChildren)
	{
		myTransformStack.push(child);
	}

	return *this;
}

GameObject* Engine::GameObjectChildIterator::DerefAsRaw() const
{
	return myCurrentTransform->GetGameObject();
}

Shared<GameObject> Engine::GameObjectChildIterator::DerefAsShared() const
{
	return myCurrentTransform->GetGameObjectShared();
}
