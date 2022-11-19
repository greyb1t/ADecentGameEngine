#include "pch.h"
#include "HealthBarComponent.h"

#include "Engine\GameObject\GameObject.h"
#include "Game/Components/HealthComponent.h"

void HealthBarComponent::Start()
{
	Transform* parent = GetTransform().GetParent();
	if(!parent)
	{
		LOG_WARNING(LogType::Game) << "No parent found.";
		return;
	}

	myHealth = parent->GetGameObject()->GetComponent<HealthComponent>();
	if(!myHealth)
	{
		LOG_WARNING(LogType::Game) << "No HealthComponent found in parent.";
	}

	mySpriteComponent = GetGameObject()->GetComponent<Engine::SpriteComponent>();
	if(!mySpriteComponent)
	{
		LOG_WARNING(LogType::Game) << "No SpriteComponent found.";
		return;
	}

	mySpriteComponent->SetActive(false);
}

void HealthBarComponent::Execute(Engine::eEngineOrder)
{
	if(!myHealth)
	{
		return;
	}

	float healthFraction = myHealth->GetHealth() / myHealth->GetMaxHealth();

	if(healthFraction <= 0.f)
	{
		GetGameObject()->Destroy();
		return;
	}

	if(mySpriteComponent)
	{
		if(healthFraction < 1.f)
		{
			mySpriteComponent->SetActive(true);
		}
		
		mySpriteComponent->GetMaterialInstance().SetFloat4("value1", {healthFraction, 0.f, 0.f, 1.f});
	}
}