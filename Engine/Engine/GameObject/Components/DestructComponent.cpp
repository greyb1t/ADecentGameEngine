#include "pch.h"
#include <Engine/GameObject/Components/ScriptsComponent.h>
#include "DestructComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/Reflection/Reflector.h"
#include "Engine/AudioManager.h"

Engine::DestructComponent::DestructComponent(GameObject* aGameObject, float aTime)
	: Component(aGameObject), myTime(aTime)
{

}

Engine::DestructComponent::~DestructComponent()
{
	
}
void Engine::DestructComponent::Start()
{
	
}
void Engine::DestructComponent::Execute(eEngineOrder aOrder)
{
	myTimer += Time::DeltaTime;
	if (myTimer >= myTime)
	{
		myGameObject->Destroy();
	}
}

void Engine::DestructComponent::Reflect(Reflector& aReflector)
{
	//Must be in all components
	Component::Reflect(aReflector);
}
