#pragma once
#include "Engine/GameObject/Components/Component.h"
#include "Engine\GameObject\GameObject.h"

class Projectile : public Component
{
public:
	COMPONENT(Projectile, "Projectile");

	Projectile() = default;
	Projectile(GameObject* aGameObject);
	virtual ~Projectile() = default;

	void Start() override;

	void Execute(Engine::eEngineOrder aOrder) override;

	void Update();

	void Reflect(Engine::Reflector& aReflector) override;

	void Render() override;

	void Init(float aSpeed, const Vec3f& aDirection, float aDamage, Engine::UUID aShooterUUID);
	bool CheckLifeTime();
	bool CheckCollisionAndMove(LayerMask aLayerMask, RayCastHit& aOutHit, bool aDrawBullet = false);
	bool CheckSphereCollision(LayerMask aLayerMask, std::vector<OverlapHit>& aOutHit, float aRadius, bool aDrawBullet = false);
	void MoveInForwardAndRotateTowardsPlayer();
	void MoveInForward();
	void SetTargetRotationRadians(float aRadians, float aRotationSmoothness);
	void RotateTowardsPlayer(float aRotationSmoothness);
	void UpdateSmoothRotation();

protected:
	float mySpeed = 15000.0f;
	Vec3f myDirection;
	float myDamage = 0.0f;

	float myLifeTime = 15.0f;
	float myLifeTimer = 0.0f;
	Vec3f myHitPos = { 0.f, 0.f, 0.f };
	Engine::UUID myShooterUUID;
private:
	float myCurrentRotationRadians = 0.f;
	float myTargetRotationRadians = 0.f;
	float myRotationSmoothness = 16.f;
	bool myActivateUpdateRotation = false;
};

