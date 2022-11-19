#include "pch.h"
#include "BouncingProjectile.h"

#include "DamageNumber.h"
#include "Components/HealthComponent.h"
#include "Engine/Shortcuts.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/Scene/Scene.h"
#include "AI/LevelBoss/Objects/BossDome.h"
#include "Items/ItemManager.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/GameObject/Components/AudioComponent.h"
#include <AI/PollingStation/PollingStationComponent.h>
#include <Player/Player.h>


BouncingProjectile::BouncingProjectile(GameObject* aObj)
	: Component(aObj)
{

}

void BouncingProjectile::Start()
{
	Component::Start();
}

void BouncingProjectile::Setup(Vec3f aDir, const DamageInfo& aDamageInfo, int aBounces, float aBounceRange, float aSpeed, float aBounceSpeed, float aRange, VFXRef aProjectileVFX, VFXRef aHitVFX)
{
	myDirection = aDir;
	myDamageInfo = aDamageInfo;
	mySpeed = aSpeed;
	myRange = aRange;
	myBounceRange = aBounceRange;
	myMaxBounces = aBounces;
	myBouncesLeft = myMaxBounces;
	myBounceSpeed = aBounceSpeed;

	myProjectileVFX = aProjectileVFX;
	myHitVFX = aHitVFX;

	Init();
}

void BouncingProjectile::Init()
{
	if (!myProjectileVFX || !myProjectileVFX->IsValid())
		return;
	auto obj = GetGameObject()->GetScene()->AddGameObject<GameObject>();
	myVFXObj = obj->GetWeak();
	obj->GetTransform().SetParent(&myGameObject->GetTransform());
	obj->GetTransform().SetPositionLocal({ 0,0,0 });
	myVFX = obj->AddComponent<Engine::VFXComponent>(myProjectileVFX->Get());
	myVFX->Play();
	myVFX->AutoDestroy();
	myAudioComponent = myGameObject->AddComponent<Engine::AudioComponent>();
	myAudioComponent->AddEvent("Bounce", "event:/SFX/PLR/BouncingAttack");

	myAudioComponent->PlayEvent("Bounce");
}

void BouncingProjectile::Hit(GameObject* aObj)
{
	LOG_INFO(LogType::Viktor) << "Hit a target!";


	const float damage = myDamageInfo.GetDamage();
	ItemEventData data;
	data.damage = damage;
	data.direction = myDirection;
	data.targetObject = aObj;
	data.hitPos = aObj->GetTransform().GetPosition();

	Main::GetItemManager().PostEvent(eItemEvent::OnShotImpact, &data);

	if (aObj->GetLayer() & eLayer::ENEMY)
	{
		Main::GetItemManager().PostEvent(eItemEvent::OnEnemyHit, &data);
		if (myDamageInfo.IsCrit())
		{
			Main::GetItemManager().PostEvent(eItemEvent::OnCrit, &data);
		}
	}

	if (aObj->GetLayer() & eLayer::NONPLAYERDAMAGEABLE)
	{
		auto hp = aObj->GetComponent<HealthComponent>();
		hp->ApplyDamage(damage, false, nullptr, myDamageInfo.IsCrit() ? eDamageType::Crit : eDamageType::Basic);

		auto* player = reinterpret_cast<Player*>(myGameObject->GetSingletonComponent<PollingStationComponent>()->GetPlayerComponent());
		player->GetCrosshair().HitMark();
		//{ // Create dmg number
		//	auto dmgObj = myGameObject->GetScene()->AddGameObject<GameObject>();
		//	auto* dmgNumber = dmgObj->AddComponent<DamageNumber>(myDamage);

		//	dmgObj->GetTransform().SetPosition(GetTransform().GetPosition() + (GetTransform().Right() * -15.f + GetTransform().Up() * -15.f + GetTransform().Forward() * -10.f));
		//}


		myTargetObject = aObj->GetWeak();
		Bounce(aObj);

		if (myHitVFX && myHitVFX->IsValid())
		{
			auto obj = GetGameObject()->GetScene()->AddGameObject<GameObject>();
			obj->GetTransform().SetPosition(GetTransform().GetPosition());
			auto vfx = obj->AddComponent<Engine::VFXComponent>(myHitVFX->Get());
			vfx->Play();
			vfx->AutoDestroy();
		}
		return;
	}
	KillInstance();
}

void BouncingProjectile::KillInstance()
{
	if (!myVFXObj.expired())
	{
		myVFX->Stop();
	}
	myGameObject->Destroy();
}

void BouncingProjectile::Bounce(GameObject* aObj)
{
	if (--myBouncesLeft < 0)
	{
		KillInstance();
	}

	myAudioComponent->PlayEvent("Bounce");

	if (auto* target = SearchNextTarget())
	{
		myTargetObject = target->GetWeak();
		myBouncing = true;
		return;
	}

	KillInstance();
}

GameObject* BouncingProjectile::SearchNextTarget()
{
	GameObject* target = nullptr;
	std::vector<OverlapHit> hits;
	auto scene = myGameObject->GetScene();
	if (scene->SphereCastAll(GetTransform().GetPosition(), myBounceRange, eLayer::NONPLAYERDAMAGEABLE, hits))
	{
		float closestDistance = 0;

		const Vec3f pos = GetTransform().GetPosition();
		std::sort(hits.begin(), hits.end(), [&](OverlapHit a, OverlapHit b)
			{
				if (!a.GameObject)
					return false;
				if (!b.GameObject)
					return true;

				const float distanceA = (a.GameObject->GetTransform().GetPosition() - pos).LengthSqr();
				const float distanceB = (b.GameObject->GetTransform().GetPosition() - pos).LengthSqr();

				return distanceA < distanceB;
			});

		for (int i = 0; i < hits.size(); i++)
		{
			auto& hit = hits[i];
			if (!myTargetObject.expired() && myTargetObject.lock()->GetUUID() == hit.UUID)
				continue;

			auto deltaPos = hit.GameObject->GetTransform().GetPosition() - pos;
			const float distance = (deltaPos).Length();
			auto dir = deltaPos;
			dir.Normalize(distance);

			RayCastHit rayhit;
			if (!scene->RayCast(pos, dir, distance, eLayer::GROUND | eLayer::DEFAULT, rayhit))
			{
				return hit.GameObject;
			}

		}
	}

	return target;
}

void BouncingProjectile::Execute(Engine::eEngineOrder aOrder)
{
	if (myBouncing)
	{
		HomingMove();
		return;
	}
	NormalMove();
}

void BouncingProjectile::NormalMove()
{
	float distance = mySpeed * Time::DeltaTime;
	LayerMask mask = eLayer::DEFAULT | eLayer::GROUND | eLayer::NONPLAYERDAMAGEABLE;

	RayCastHit hit;
	if (myGameObject->GetScene()->RayCast(GetTransform().GetPosition(), myDirection, distance, mask, hit))
	{

		if (hit.Layer & eLayer::PLAYER_PROJECTILE_BLOCKING)
		{
			if (auto dome = hit.GameObject->GetComponent<BossDome>())
			{
				dome->OnHit(hit, myGameObject);
			}
		}

		GetTransform().SetPosition(hit.Position);
		Hit(hit.GameObject);
		return;
	}

	const Vec3f movement = myDirection * distance;

	GDebugDrawer->DrawLine3D(DebugDrawFlags::Always, GetTransform().GetPosition(), GetTransform().GetPosition() + movement, .3f, Vec4f(0, 0, 1.f, .7f));

	GetTransform().Move(movement);

	myDistanceTraveled += distance;
	if (myDistanceTraveled > myRange)
		KillInstance();
}

void BouncingProjectile::HomingMove()
{
	if (myTargetObject.expired())
	{
		myBouncing = false;
		return;
	}

	const auto target = myTargetObject.lock();

	float moveDistance = mySpeed * Time::DeltaTime;

	Vec3f deltaPos = target->GetTransform().GetPosition() - GetTransform().GetPosition();

	float length = deltaPos.Length();
	if (length <= moveDistance)
	{
		GetTransform().SetPosition(target->GetTransform().GetPosition());
		Hit(target.get());
		return;
	}

	deltaPos.Normalize(length);
	Vec3f movement = deltaPos * moveDistance;
	GetTransform().Move(movement);

	GDebugDrawer->DrawLine3D(DebugDrawFlags::Always, GetTransform().GetPosition(), GetTransform().GetPosition() + movement, .3f, Vec4f(0, 0, 1.f, .7f));
}