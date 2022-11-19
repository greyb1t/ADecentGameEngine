#pragma once

#include "Engine/GameObject/Components/Component.h"

namespace Engine
{
	class EnvironmentLightComponent;
	struct FogData;
}

class FinalBossLightning : public Component
{
public:
	COMPONENT(FinalBossLightning, "FB Lightning");

	enum State
	{
		IntensityGoUp,
		IntensityGoDown,
		WaitCooldown,
	};

	FinalBossLightning();
	FinalBossLightning(GameObject* aGameObject);
	~FinalBossLightning();

	void Start() override;

	void Execute(Engine::eEngineOrder aOrder) override;

	void Reflect(Engine::Reflector& aReflector) override;

private:
	Engine::EnvironmentLightComponent* myEnvironmentLight = nullptr;
	Engine::FogData* myFog = nullptr;

	Vec3f myInitialFogColor;
	Vec3f myTargetFogColor = Vec3f(1.f, 1.f, 1.f);

	float myIntensityTimer = 0.f;
	float myIntensityDuration = 0.f;

	float myCooldownTimer = 0.f;
	float myCooldownDuration = 0.f;

	// A random value between these 2 will get generated to know how quickly
	// the light will lerp intensity
	Vec2f myIntensityDurationMinMax = Vec2f(0.04f, 0.09f);

	// A random value between will determine how long to wait
	// until next intensity lerp
	Vec2f myCooldownDurationMinMax = Vec2f(0.07f, 2.f);

	State myState = State::WaitCooldown;
};

