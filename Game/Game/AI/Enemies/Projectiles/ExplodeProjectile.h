#pragma once
#include "Projectile.h"
#include "Engine\GameObject\GameObject.h"

class ProjectileHealth;

class ExplodeProjectile : public Projectile
{
public:
	COMPONENT(ExplodeProjectile, "ExplodeProjectile");

	ExplodeProjectile() = default;
	ExplodeProjectile(GameObject* aGameObject, float aRadius, float aExplosionRadius);
	virtual ~ExplodeProjectile() = default;

	void Start() override;

	void Execute(Engine::eEngineOrder aOrder) override;

	void Update();

	void Reflect(Engine::Reflector& aReflector) override;

	void Render() override;

	void OnCollision();

	void CreateSphereCast();
	void ExplosionCreateSphereCast(bool aShouldDealDamage = true);

	void SpawnExplosionSound();

private:
	ProjectileHealth* myProjectileHealth = nullptr;
	float myRadius = 30.f;
	float myExplosionRadius = 250.f;

	Engine::VFXComponent* myVFX = nullptr;
	GameObject* myModel = nullptr;
	int mySphereCastFrameCount = 0;
	int mySphereCastFrameCooldown = 5;
};

