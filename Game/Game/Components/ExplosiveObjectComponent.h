#pragma once
#include "Engine\GameObject\Components\Component.h"
#include "Engine\ResourceManagement\ResourceRef.h"

namespace Engine
{
	class VFXComponent;
}

class HealthComponent;

class ExplosiveObjectComponent : public Component
{
	COMPONENT(ExplosiveObjectComponent, "Explosive Object Component");
public:
	ExplosiveObjectComponent() = default;
	ExplosiveObjectComponent(GameObject * aGameObject);
	~ExplosiveObjectComponent();

	void Start() override;
	void Setup(const float& aHealth);
	void Reflect(Engine::Reflector& aReflector) override;

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
};

