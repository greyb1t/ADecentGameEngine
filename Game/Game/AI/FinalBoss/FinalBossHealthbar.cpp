#include "pch.h"
#include "FinalBossHealthbar.h"
#include "Engine/GameObject/GameObject.h"
#include "Components/HealthComponent.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"

void FB::FinalBossHealthbar::Start()
{
	HealthBarComponent::Start();

	if (!myHealthTargetGameObject || !myHealthTargetGameObject.IsValid())
	{
		myGameObject->Destroy();
		LOG_ERROR(LogType::Game) << "Missing health target gameobject";
		return;
	}

	myHealth = myHealthTargetGameObject.Get()->GetComponent<HealthComponent>();

	if (!myHealth)
	{
		LOG_WARNING(LogType::Game) << "No HealthComponent found in target gameobject.";
		return;
	}
}

void FB::FinalBossHealthbar::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myHealthTargetGameObject, "Health Target GameObject");
}
