#pragma once
#include "Engine\GameObject\Components\Component.h"
#include "Engine\ResourceManagement\ResourceRef.h"

namespace Engine
{
	class VFXComponent;
}

class HealthComponent;

class ExplosionComponent : public Component
{
	COMPONENT(ExplosionComponent, "Explosion Component");
public:
	ExplosionComponent() = default;
	ExplosionComponent(GameObject * aGameObject);
	~ExplosionComponent();

	void Start() override;
	void Setup(const float& aHealth);
	void Reflect(Engine::Reflector& aReflector) override;

	void Execute(Engine::eEngineOrder aOrder) override;

	void Explode();

private:
	void OnDeath();

	float myExplosionRadius = 500.0f;
	float myDamage = 40.0f;
	float myMaxHealth = 30.0f;

	HealthComponent* myHealthComponent = nullptr;

	Engine::VFXComponent* myExplosionVFX = nullptr;
	VFXRef myExplosionRef;

	GameObject* myModelChild = nullptr;

	bool myHasPlayedDeath = false;

	bool myHasExploded = false;

	float myDeathTimer = 0.f;
	float myDeathTime = 4.f;
};

