#pragma once
#include "Projectile.h"
#include "Engine\GameObject\GameObject.h"

class PopcornProjectile : public Projectile
{
public:
	COMPONENT(PopcornProjectile, "PopcornProjectile");

	PopcornProjectile() = default;
	PopcornProjectile(GameObject* aGameObject);
	virtual ~PopcornProjectile() = default;

	void Start() override;

	void Execute(Engine::eEngineOrder aOrder) override;

	void Update();

	void Reflect(Engine::Reflector& aReflector) override;

	void Render() override;
	void CreateSphereCast() const;

private:
	Engine::VFXComponent* myVFX = nullptr;
	GameObject* myModel = nullptr;
	int mySphereCastFrameCount = 0;
	int mySphereCastFrameCooldown = 3;
};

