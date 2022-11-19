#include "pch.h"
#include "Component.h"

#include "../GameObject.h"
#include "Engine/Reflection/Reflector.h"

Component::Component(GameObject* aGameObject)
{
	myGameObject = aGameObject;
}

Component::~Component()
{
}

void Component::SetActive(bool aValue)
{
	if (aValue)
	{
		if (!myHasStarted)
		{
			InternalAwake();
			InternalStart();
		}
	}

	if (aValue && !myIsActive)
	{
		OnEnable();
	}
	else if (!aValue && myIsActive)
	{
		OnDisable();
	}
	myIsActive = aValue;
}

bool Component::HasStarted() const
{
	return myHasStarted;
}

GameObject* Component::GetGameObject()
{
	return myGameObject;
}

const GameObject* Component::GetGameObject() const
{
	return myGameObject;
}

Transform& Component::GetTransform()
{
	return myGameObject->GetTransform();
}

const Transform& Component::GetTransform() const
{
	return myGameObject->GetTransform();
}

void Component::ChangeParentGameObject(GameObject& aNewGameObject)
{
	myGameObject = &aNewGameObject;
}

void Component::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myIsActive, "Is Active");
}

int Component::GetComponentTypeID() const
{
	return myID;
}

void Component::InternalStart()
{
	if (myHasStarted)
	{
		return;
	}

	myHasStarted = true;

	//Awake();
	Start();
}

void Component::InternalAwake()
{
	if (myHasAwaked)
	{
		return;
	}

	myHasAwaked = true;

	Awake();
}

bool Component::IsDestroyed() const
{
	return myIsDestroyed;
}

void Component::Destroy()
{
	myIsDestroyed = true;
}

bool Component::IsActive() const
{
	return myIsActive;
}
