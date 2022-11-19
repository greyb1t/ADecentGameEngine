#pragma once

#include "Common/Types.h"
#include "Engine/GameObject/Components/Component.h"
#include "Engine/GameObject/Components/SpriteComponent.h"
#include "Engine/EngineOrder.h"

class HealthComponent;

class HealthBarComponent : public Component
{
public:
	COMPONENT(HealthBarComponent, "HealthBarComponent");

	virtual void Start() override;
	void Execute(Engine::eEngineOrder) override;

protected:
	Engine::SpriteComponent* mySpriteComponent = nullptr;
	HealthComponent* myHealth = nullptr;
};