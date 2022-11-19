#pragma once
#include "Engine/GameObject/Components/Component.h"
#include "Engine\ResourceManagement\ResourceRef.h"

class HealthComponent;

class ProjectileHealth : public Component
{
public:
	ProjectileHealth();
	ProjectileHealth(GameObject* aGameObject);
	~ProjectileHealth() override;

	void Init(float aMaxHealth, float aExplosionRadius, float aExplosionDamage);
	void SetDeathFunction(std::function<void()> aFunction);

	Component* Clone() const override { return DBG_NEW ProjectileHealth(*this); }
	void Reflect(Engine::Reflector& aReflector) override;

	void Start() override;

	void Execute(Engine::eEngineOrder aOrder) override;

	void OnDamage(float aDamage, const Vec3f& aDamageDir);
	void OnDeath(float aDamage);

	HealthComponent* GetHealthComponent();

private:
	HealthComponent* myHealthComponent = nullptr;
	float myMaxHealth = 0.f;
	float myExplosionRadius = 200.f;
	float myExplosionDamage = 2.f;

	std::function<void()> myFunction = nullptr;
};

