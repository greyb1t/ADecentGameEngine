#pragma once
#include "Engine\GameObject\Components\Component.h"
#include <Engine/Reflection/Reflector.h>

struct RayCastHit;

class HomingProjectile :
	public Component
{
public:
	COMPONENT(HomingProjectile, "HomingProjectile")

	HomingProjectile() = default;
	HomingProjectile(GameObject* aGameObject);

	void Setup(const Vec3f& aDir, float aSpeed, float aDmg, float aRange, float rotationSpeed, float aRadius);

	void Execute(Engine::eEngineOrder aOrder) override;

	void Reflect(Engine::Reflector& aReflector) override;
	void OnHit(RayCastHit& hit);

private:
	Weak<GameObject> SearchNextTarget();

	Weak<GameObject> myTarget{};
	
	Vec3f myDirection;
	Vec3f myWantedDirection;
	Vec3f myMovement;
	float mySpeed = 100;
	float myDamage = 10;
	float myRange = 5000;
	float myDistanceTraveled = 0;
	float myRotationProgress = 0;
	float myRotationSpeedMultiplier = 1.0f;
	float myRadius = 0;
};

