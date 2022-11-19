#include "pch.h"
#include "HomingProjectile.h"

#include "DamageNumber.h"
#include "Components/HealthComponent.h"
#include "Engine/Shortcuts.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/VFX/VFXUtilities.h"
#include "Game/AI/LevelBoss/Objects/BossDome.h"

#include "Items/ItemManager.h"
#include <AI/PollingStation/PollingStationComponent.h>
#include "Engine/GameObject/Components/ModelComponent.h"

HomingProjectile::HomingProjectile(GameObject* aObject)
	: Component(aObject)
{
}

void HomingProjectile::Setup(const Vec3f& aDir, float aSpeed, const float aDmg, float aRange, float rotationSpeedMultiplier, float aRadius)
{
	myDirection = aDir.GetNormalized();
	mySpeed = aSpeed;
	myDamage = aDmg;
	myRange = aRange;
	myRotationSpeedMultiplier = rotationSpeedMultiplier;
	myRadius = aRadius;

	myTarget = SearchNextTarget();
	if(myTarget.expired())
	{
		GetGameObject()->Destroy();
	}
}

void HomingProjectile::Reflect(Engine::Reflector& aReflector)
{
	Component::Reflect(aReflector);
}

void HomingProjectile::Execute(Engine::eEngineOrder aOrder)
{
	Shared<GameObject> target = myTarget.lock();

	if(!target)
	{
		myTarget = SearchNextTarget();
		if(myTarget.expired())
		{
			GetGameObject()->Destroy();
		}

		return;
	}

	HealthComponent* healthComponent = target->GetComponent<HealthComponent>();
	if(healthComponent)
	{
		if(healthComponent->HasNoHealth())
		{
			myTarget = {};
			return;
		}
	}

	Vec3f targetDelta = target->GetTransform().GetPosition() - GetTransform().GetPosition();

	static constexpr float steeringStrength = 1.f;
	myDirection += targetDelta.GetNormalized() * steeringStrength;
	myDirection.Normalize();

	Vec3f velocity = myDirection * mySpeed * Time::DeltaTime;
	GetTransform().SetPosition(GetTransform().GetPosition() + velocity);

	static constexpr float minDistance = 50.f;
	if (targetDelta.Length() <= minDistance)
	{
		healthComponent->ApplyDamage(myDamage);
		GetGameObject()->Destroy();
	}

	Quatf rot;
	rot.InitWithAxisAndRotation({0, 1, 0}, 14.0f * Time::DeltaTime);
	myGameObject->GetTransform().Rotate(rot);
}

void HomingProjectile::OnHit(RayCastHit& hit)
{
	if (hit.Layer & eLayer::DAMAGEABLE)
	{
		auto hp = hit.GameObject->GetComponent<HealthComponent>();
		hp->ApplyDamage(myDamage);

		{ // Create dmg number
			auto obj = myGameObject->GetScene()->AddGameObject<GameObject>();
			auto* dmgNumber = obj->AddComponent<DamageNumber>(myDamage);

			obj->GetTransform().SetPosition(GetTransform().GetPosition() + (GetTransform().Right() * -15.f + GetTransform().Up() * -15.f + GetTransform().Forward() * -10.f));
		}
	}

	ItemEventData data;
	data.targetObject = hit.GameObject;
	data.damage = myDamage;
	data.hitPos = hit.Position;
	data.direction = myDirection;

	Main::GetItemManager().PostEvent(eItemEvent::OnShotImpact, &data);

	if (hit.Layer & eLayer::ENEMY)
	{
		Main::GetItemManager().PostEvent(eItemEvent::OnEnemyHit, &data);
	}
	if (hit.Layer & eLayer::PLAYER_PROJECTILE_BLOCKING)
	{
		if (auto dome = hit.GameObject->GetComponent<BossDome>())
		{
			dome->OnHit(hit, myGameObject);
		}
	}
	GDebugDrawer->DrawSphere3D(DebugDrawFlags::Always, GetTransform().GetPosition(), 3, .5f, Vec4f(1, 0.f, 0.f, 1.f));


	//auto* vfx = myGameObject->GetScene()->AddGameObject<GameObject>();
	//auto vfxComponent = vfx->AddComponent<Engine::VFXComponent>(VFX::Load("Assets/VFX/EnemyBulletHit.vfx"));
	//vfxComponent->Play();
	//vfxComponent->AutoDestroy();
}

Weak<GameObject> HomingProjectile::SearchNextTarget()
{
	std::vector<OverlapHit> hits;
	auto scene = myGameObject->GetScene();
	if (scene->SphereCastAll(GetTransform().GetPosition(), myRange, eLayer::NONPLAYERDAMAGEABLE, hits))
	{
		const Vec3f pos = GetTransform().GetPosition();
		std::sort(hits.begin(), hits.end(), [&](OverlapHit a, OverlapHit b) -> bool
		{
			if (!a.GameObject)
				return false;
				
			if (!b.GameObject)
				return true;

			const float distanceA = (a.GameObject->GetTransform().GetPosition() - pos).LengthSqr();
			const float distanceB = (b.GameObject->GetTransform().GetPosition() - pos).LengthSqr();

			return distanceA < distanceB;
		});

		for(const OverlapHit& hit : hits)
		{
			Weak<GameObject> goPtr = hit.GameObject->GetWeak();
			Shared<GameObject> gameObject = goPtr.lock();

			if(!gameObject)
				continue;
			
			if(gameObject->IsDestroyed())
				continue;
			
			return goPtr;
		}
	}

	return {};
}