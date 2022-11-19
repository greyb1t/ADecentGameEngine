#include "pch.h"
#include "Projectile.h"

#include "AI/PollingStation/PollingStationComponent.h"
#include "Engine/Shortcuts.h"

Projectile::Projectile(GameObject* aGameObject) :
	Component(aGameObject)
{
}

void Projectile::Start()
{
	Component::Start();
}

void Projectile::Execute(Engine::eEngineOrder aOrder)
{
	Component::Execute(aOrder);
	Update();
}

void Projectile::Update()
{
	if (myActivateUpdateRotation)
		UpdateSmoothRotation();

	/*GDebugDrawer->DrawSphere3D(
		DebugDrawFlags::AI,
		myGameObject->GetTransform().GetPosition(),
		5.f,
		0.f,
		{1.f, 0.f, 1.f, 1.f});*/
}

void Projectile::Reflect(Engine::Reflector& aReflector)
{
	Component::Reflect(aReflector);
}

void Projectile::Render()
{
	Component::Render();
}	

void Projectile::Init(float aSpeed, const Vec3f& aDirection, float aDamage, Engine::UUID aShooterUUID)
{
	mySpeed = aSpeed;
	myDirection = aDirection;
	myDamage = aDamage;
	myShooterUUID = aShooterUUID;
}

bool Projectile::CheckLifeTime()
{
	myLifeTimer += Time::DeltaTime;
	if (myLifeTimer > myLifeTime)
	{
		return false;
	}

	return true;
}

bool Projectile::CheckCollisionAndMove(LayerMask aLayerMask, RayCastHit& aOutHit, bool aDrawBullet)
{
	bool hitSomeThing = false;

	Vec3f position = myGameObject->GetTransform().GetPosition();
	const float distance = mySpeed * Time::DeltaTime;

	if (myGameObject->GetScene()->RayCast(position, myDirection, distance, aLayerMask, aOutHit) == true)
	{
		if (aOutHit.GameObject != nullptr)
		{
			myHitPos = position + myDirection * distance;
		}
		hitSomeThing = true;
	}

	position += myDirection * distance;
	myGameObject->GetTransform().SetPosition(position);


	if (aDrawBullet)
		GDebugDrawer->DrawLine3D(DebugDrawFlags::AI, position, position - myDirection * distance, 2.0f);


	return hitSomeThing;
}

bool Projectile::CheckSphereCollision(LayerMask aLayerMask, std::vector<OverlapHit>& aOutHit, float aRadius, bool aDrawBullet)
{
	bool hitSomeThing = false;

	Vec3f position = myGameObject->GetTransform().GetPosition();
	float distance = mySpeed * Time::DeltaTime;

	if (myGameObject->GetScene()->SphereCastAll(position, aRadius, aLayerMask, aOutHit) == true)
	{
		for (int hitIndex = 0; hitIndex < aOutHit.size(); hitIndex++)
		{
			if (aOutHit[hitIndex].GameObject != nullptr)
			{
				myHitPos = position;
				break;
			}
		}
		hitSomeThing = true;
	}

	if (aDrawBullet)
		GDebugDrawer->DrawSphere3D(DebugDrawFlags::AI, position, aRadius);

	return hitSomeThing;
}

void Projectile::MoveInForwardAndRotateTowardsPlayer()
{
	myActivateUpdateRotation = true; // Shit Solution but works ;)

	Vec3f playerPos = myGameObject->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos();
	playerPos.y -= 100.f;
	myGameObject->GetTransform().LookAt(playerPos);

	auto moveDir = myGameObject->GetTransform().Forward();
	moveDir.Normalize();

	RotateTowardsPlayer(16.f);

	const auto movement = moveDir * mySpeed * Time::DeltaTime;
	myGameObject->GetTransform().Move(movement);
}

void Projectile::MoveInForward()
{
	myActivateUpdateRotation = true; // Shit Solution but works ;)

	auto moveDir = myGameObject->GetTransform().Forward();
	moveDir.Normalize();

	const float speed = 300.f;
	const auto movement = moveDir * speed * Time::DeltaTime;
	myGameObject->GetTransform().Move(movement);
}

void Projectile::SetTargetRotationRadians(float aRadians, float aRotationSmoothness)
{
	myRotationSmoothness = aRotationSmoothness;
	myTargetRotationRadians = aRadians;
}

void Projectile::RotateTowardsPlayer(float aRotationSmoothness)
{
	auto targetPos = myGameObject->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos();
	auto moveDir = targetPos - GetGameObject()->GetTransform().GetPosition();
	moveDir.Normalize();

	float angle = atan2(moveDir.z, moveDir.x);
	SetTargetRotationRadians(angle + 90.f * Math::Deg2Rad, aRotationSmoothness);
}

void Projectile::UpdateSmoothRotation()
{
	myCurrentRotationRadians = Math::LerpAngleRadians(
		myCurrentRotationRadians,
		myTargetRotationRadians,
		Time::DeltaTime * myRotationSmoothness);

	GetGameObject()->GetTransform().SetRotation(CU::Vector3f(0.0f, myCurrentRotationRadians, 0.0f));
}