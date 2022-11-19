#pragma once

#include "Engine\GameObject\Components\Component.h"

class LaunchpadComponent final: public Component
{
public:
	COMPONENT(LaunchpadComponent, "LaunchpadComponent");

	LaunchpadComponent() = default;
	LaunchpadComponent(GameObject* aObject);

	void Start() override;
	void Execute(Engine::eEngineOrder) override;
	void Reflect(Engine::Reflector& aReflector) override;

private:
	void Collide(GameObject* aObject);

	void InitAudio();

	float myStrength = 300.f;
	float myCollisionTimer = 0.f;
};