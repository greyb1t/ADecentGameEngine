#pragma once

#include "Engine/GameObject/Components/Component.h"

class DestroyAfterSecondsComponent : public Component
{
public:
	COMPONENT(DestroyAfterSecondsComponent, "DestroyAfterSecondsComponent");

	void Start();
	void Execute(Engine::eEngineOrder) override;
	void Reflect(Engine::Reflector& aReflector) override;

private:
	float myTimer = 0.f;
	float myDuration = 0.f;
};