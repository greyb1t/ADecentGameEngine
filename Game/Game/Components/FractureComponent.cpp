#include "pch.h"
#include "FractureComponent.h"

#include "Engine/GameObject/GameObject.h"

FractureComponent::FractureComponent(GameObject* aGameObject, float aLifeTime, float aDisappearThreshold, float aLifetimeRandomMagnitude)
	: Component(aGameObject),
	myDisappearThreshold(aDisappearThreshold),
	myRandomizeModifier(aLifetimeRandomMagnitude),
	myOriginalScale(myGameObject->GetTransform().GetScaleLocal())
{
	myLifeTime = aLifeTime + (aLifeTime * Random::RandomFloat(-myRandomizeModifier, myRandomizeModifier));
}

void FractureComponent::Execute(Engine::eEngineOrder aOrder)
{
	myTime += Time::DeltaTime;

	const float percentage = (myLifeTime - myTime) / (myLifeTime - (myLifeTime * myDisappearThreshold));
	if (percentage < 1.f)
	{
		if (percentage <= 0.f)
		{
			myGameObject->Destroy();
			return;
		}
		myGameObject->GetTransform().SetScaleLocal(myOriginalScale * percentage);

	}
}
