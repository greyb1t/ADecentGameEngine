#pragma once
#include "Engine/GameObject/Components/Component.h"

class PoisonCloudDamageComponent : public Component
{
	COMPONENT(PoisonCloudDamageComponent, "PoisonCloudDamageComponent");

public:
	PoisonCloudDamageComponent() = default;
	PoisonCloudDamageComponent(GameObject* anObject);

	void Execute(Engine::eEngineOrder aOrder) override;

private:
	const float myDmgDist = 300.f;
	const float myLifeTime = 10.0f;
	const float myTicksPerSecond = 2.f;
	const float myTickDamage = 3.f;

	float myDuration = 0.0f;
	float myTickProgress = 0.0f;
};