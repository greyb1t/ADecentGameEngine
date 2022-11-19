#pragma once
#include "Projectile.h"
#include "Engine\GameObject\GameObject.h"

class ProjectileHealth;

class BurstTankProjectile : public Projectile
{
public:
	COMPONENT(BurstTankProjectile, "BurstTankProjectile");

	BurstTankProjectile() = default;
	BurstTankProjectile(GameObject* aGameObject);
	virtual ~BurstTankProjectile() = default;

	void Start() override;

	void Execute(Engine::eEngineOrder aOrder) override;

	void Update();

	void Reflect(Engine::Reflector& aReflector) override;

	void Render() override;

	void OnCollision();
private:
	ProjectileHealth* myProjectileHealth = nullptr;
};

