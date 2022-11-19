#include "pch.h"
#include "DestroyAfterSecondsComponent.h"

#include "Engine/GameObject/GameObject.h"

void DestroyAfterSecondsComponent::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myDuration, "Duration");
}

void DestroyAfterSecondsComponent::Start()
{
	myTimer = myDuration;
}

void DestroyAfterSecondsComponent::Execute(Engine::eEngineOrder)
{
	myTimer -= Time::DeltaTime;
	if(myTimer <= 0.f)
	{
		GetGameObject()->Destroy();
	}
}
