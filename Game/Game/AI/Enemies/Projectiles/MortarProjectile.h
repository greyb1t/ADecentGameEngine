#pragma once
#include "Projectile.h"
#include "Engine\GameObject\GameObject.h"

class ProjectileHealth;

class MortarProjectile : public Projectile
{
public:
	COMPONENT(MortarProjectile, "MortarProjectile");

	MortarProjectile() = default;
	MortarProjectile(GameObject* aGameObject, float aProjectileRadius, float aExplosionRadius);
	virtual ~MortarProjectile() = default;

	void Start() override;

	void Execute(Engine::eEngineOrder aOrder) override;

	void Update();

	void Reflect(Engine::Reflector& aReflector) override;

	void Render() override;

	//void OnCollision();

	void Explosion(bool aSouldDealDamage = true);
	void CreateSphereCast();
	void RotateTowardsPosition();
	bool CurveMovement();
	void DropMovement();
	void DestroyAll();
	void DestroyUglyFix();

private:
	ProjectileHealth* myProjectileHealth = nullptr;
	float myRadius = 30.f;
	float myExplosionRadius = 75.f;

	float myPeekTimer = 0.f;
	float myPeekReached = 0.f;

	Vec3f myTargetPosition = { 0.f, 0.f, 0.f };
	Vec3f myStartPosition = { 0.f, 0.f, 0.f };
	bool myTick = true;

	float myflyTime = 0.f;
	float myFlyProgress = 0.f;
	float myFlyDuration = 3.f;

	Engine::VFXComponent* myVFX = nullptr;
	Weak<GameObject> myVFXObj;
	GameObject* myModel = nullptr;
	GameObject* myindicator = nullptr;

	GameObjectPrefabRef myBulletHitDecal;
	MaterialRef myMatRef;
	GameObject* myDecalGO = nullptr;
};

