#pragma once
#include "Engine\GameObject\GameObject.h"

class Bullet
{
public:
	Bullet();

	void Init(GameObject* aGameObject, float aSpeed, const Vec3f& aDirection, float aDamage);

protected:
	//Return false when lifeTime is out
	bool CheckLifeTime(float aDeltaTime);

	//Raycasts and then moves the bullet
	bool CheckCollisionAndMove(float aDeltaTime, LayerMask aLayerMask, RayCastHit& aOutHit, bool aDrawBullet = false);

	float mySpeed = 15000.0f;
	Vec3f myDirection;

	float myLifeTime = 5.0f;
	float myLifeTimer = 0.0f;

	float myDamage = 0.0f;

	Vec3f myHitPos = { 0.f, 0.f, 0.f };
private:
	GameObject* myBulletGameObject = nullptr;
};
