#include "pch.h"
#include "Bullet.h"

#include "Components/HealthComponent.h"
#include "Engine/Shortcuts.h"

#include <Engine\Scene\Scene.h>

Bullet::Bullet()
{
}

void Bullet::Init(GameObject* aGameObject, float aSpeed, const Vec3f& aDirection, float aDamage)
{
	myBulletGameObject = aGameObject;
	mySpeed = aSpeed;
	myDirection = aDirection;
	myDamage = aDamage;
}

bool Bullet::CheckLifeTime(float aDeltaTime)
{
	myLifeTimer += aDeltaTime;
	if (myLifeTimer > myLifeTime)
	{
		return false;
	}

	return true;
}

bool Bullet::CheckCollisionAndMove(float aDeltaTime, LayerMask aLayerMask, RayCastHit& aOutHit, bool aDrawBullet)
{
	bool hitSomeThing = false;

	Vec3f position = myBulletGameObject->GetTransform().GetPosition();
	float distance = mySpeed * aDeltaTime;

	if (myBulletGameObject->GetScene()->RayCast(position, myDirection, distance, aLayerMask, aOutHit) == true)
	{
		if (aOutHit.GameObject != nullptr)
		{
			myHitPos = position + myDirection * distance;
		}
		hitSomeThing = true;
	}

	position += myDirection * distance;
	myBulletGameObject->GetTransform().SetPosition(position);


	if (aDrawBullet)
		GDebugDrawer->DrawLine3D(DebugDrawFlags::Gameplay, position, position - myDirection * distance, 2.0f);


	return hitSomeThing;
}
