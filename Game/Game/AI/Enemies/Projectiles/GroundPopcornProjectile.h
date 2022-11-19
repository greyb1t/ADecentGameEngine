#pragma once
#include "Projectile.h"
#include "Engine\GameObject\GameObject.h"

class GroundPopcornProjectile : public Projectile
{
public:
	COMPONENT(GroundPopcornProjectile, "GroundPopcornProjectile");

	GroundPopcornProjectile() = default;
	GroundPopcornProjectile(GameObject* aGameObject);
	virtual ~GroundPopcornProjectile() = default;

	void Start() override;

	void Execute(Engine::eEngineOrder aOrder) override;

	void Update();

	void SetRadius(float aRadius);

	void Reflect(Engine::Reflector& aReflector) override;

	void Render() override;

	void CreateSphereCast();
	void CreateSphereCast2();
	void Move();

private:
	float myRadius = 0;
	Engine::VFXComponent* myVFX = nullptr;
	int mySphereCastFrameCount = 0;
	int mySphereCastFrameCooldown = 2;
};

