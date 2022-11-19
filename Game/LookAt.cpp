#include "pch.h"
#include "LookAt.h"

#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/GameObjectRef.h"

void LookAt::Start()
{
	if (myLookAtPlayer)
	{
		myLookAtObject = Main::GetPlayer();
	}
	else 
	{
		if (myLookAtObjectRef && myLookAtObjectRef.Get())
		{
			myLookAtObject = myLookAtObjectRef.Get();
		}
	}
}

void LookAt::Execute(Engine::eEngineOrder aOrder)
{
	if (!myLookAtObject)
		return;

	auto& transform = myGameObject->GetTransform();
	transform.LookAt(myLookAtObject->GetTransform().GetPosition());
}

void LookAt::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myLookAtPlayer, "Look At Player");
	if (!myLookAtPlayer)
	{
		aReflector.Reflect(myLookAtObjectRef, "Look At Object");
	}
}
