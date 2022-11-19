#include "pch.h"
#include "PiercingComponent.h"

#include "DamageNumber.h"
#include "Components/HealthComponent.h"
#include "Engine/Shortcuts.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/GameObject/Components/AudioComponent.h"
#include "Engine/VFX/VFXUtilities.h"
#include "AI/Health/EnemyHealth.h"
#include "AI/LevelBoss/Objects/BossDome.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/ResourceManagement/Resources/GameObjectPrefabResource.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"
#include "Items/ItemManager.h"
#include "Player/Projectile/ProjectileHelper.h"
#include <AI/PollingStation/PollingStationComponent.h>
#include <Player/Player.h>
#include "Explosion.h"

PiercingComponent::PiercingComponent()
{
}

PiercingComponent::PiercingComponent(GameObject* aGameObject)
	: Component(aGameObject)
{
}

void PiercingComponent::Start()
{
}

void PiercingComponent::Setup(Vec3f aDir, const DamageInfo& aDamageInfo, float aDmgRadius, float aSpeed, float aRange, float aExplosionRange, float aExplosionDamageMultiplier, VFXRef aProjectileVFX, VFXRef aHitVFX, VFXRef aExplosionVFX, GameObjectPrefabRef aExplosionPrefab, AnimationCurveRef aExplosionSizeCurve, float aExplosionTime)
{
	myDirection = aDir;
	mySpeed = aSpeed;
	myDamageInfo = aDamageInfo;
	myDamageRadius = aDmgRadius;
	myRange = aRange;
	myExplosionRange = aExplosionRange;
	myExplosionDamageMultiplier = aExplosionDamageMultiplier;

	myProjectileVFX = aProjectileVFX;
	myHitVFX = aHitVFX;
	myExplosionVFX = aExplosionVFX;

	myExplosionTime = aExplosionTime;
	myExplosionPrefab = aExplosionPrefab;
	myExplosionSizeCurve = aExplosionSizeCurve;

	GetTransform().SetRotation(Quatf(Mat4f::LookAt(Vec3f(0, 0, 0), aDir)) * Quatf(Vec3f(0, 90.f * Math::Deg2Rad, 0)));

	Init();
}

void PiercingComponent::Execute(Engine::eEngineOrder aOrder)
{
	float distance = mySpeed * Time::DeltaTime;

	Vec3f outPosition;
	std::vector<RayCastHit> hits;
	if (ProjectileHelper::ScanProjectile(GetGameObject()->GetScene(), GetTransform().GetPosition(), myDirection, distance, myDamageRadius, eLayer::DEFAULT | eLayer::GROUND, eLayer::NONPLAYERDAMAGEABLE, 5, hits, outPosition))
	{
		for (int i = 0; i < hits.size(); i++) 
		{
			OnHit(hits[i]);
		}
		GetTransform().SetPosition(outPosition);
	}
	else {
		GetTransform().SetPosition(outPosition);

		Explode();

		KillInstance();
	}



	//RayCastHit groundHit;
	/*if (myGameObject->GetScene()->RayCast(GetTransform().GetPosition(), myDirection, distance, eLayer::DEFAULT | eLayer::GROUND, groundHit))
	{
		ScanDamage(groundHit.Distance);
		GetTransform().SetPosition(groundHit.Position);

		Explode();

		KillInstance();
	} else
	{
		ScanDamage(distance);
	}*/

	/*const Vec3f movement = myDirection * distance;
	//GDebugDrawer->DrawLine3D(DebugDrawFlags::Always, GetTransform().GetPosition(), GetTransform().GetPosition() + movement, .3f, Vec4f(0, 0, 1.f, .7f));
	//GDebugDrawer->DrawSphere3D(DebugDrawFlags::Always, GetTransform().GetPosition(), 15.f, .3f, Vec4f(0, 0, 1.f, .7f));
	GetTransform().Move(movement);*/

	myDistanceTraveled += distance;
	if (myDistanceTraveled > myRange)
		KillInstance();
}

void PiercingComponent::OnHit(RayCastHit& hit)
{
	auto it = std::find(myHits.begin(), myHits.end(), hit.UUID);
	if (it != myHits.end())
		return;

	myHits.emplace_back(hit.UUID);

	const float damage = myDamageInfo.GetDamage();

	ItemEventData data;
	data.targetObject = hit.GameObject;
	data.damage = damage;
	data.direction = myDirection;
	data.hitPos = hit.Position;

	Main::GetItemManager().PostEvent(eItemEvent::OnShotImpact, &data);

	if (hit.Layer & eLayer::ENEMY)
	{
		Main::GetItemManager().PostEvent(eItemEvent::OnEnemyHit, &data);
		if (myDamageInfo.IsCrit())
		{
			Main::GetItemManager().PostEvent(eItemEvent::OnCrit, &data);
		}
	}
	if (hit.Layer & eLayer::NONPLAYERDAMAGEABLE)
	{
		auto hp = hit.GameObject->GetComponent<HealthComponent>();
		hp->ApplyDamage(damage, false, &hit.Position, myDamageInfo.IsCrit() ? eDamageType::Crit : eDamageType::Basic);

		Player* player = nullptr;
		if (auto* pollingStation = myGameObject->GetSingletonComponent<PollingStationComponent>())
			player = reinterpret_cast<Player*>(pollingStation->GetPlayerComponent());
		if (player)
			player->GetCrosshair().HitMark();

		if (myHitVFX && myHitVFX->IsValid())
		{
			auto obj = GetGameObject()->GetScene()->AddGameObject<GameObject>();
			obj->GetTransform().SetPosition(hit.Position);
			auto vfx = obj->AddComponent<Engine::VFXComponent>(myHitVFX->Get());
			vfx->Play();
			vfx->AutoDestroy();
		}

		//hp->ApplyStun(4.f);
	}
	if (hit.Layer & eLayer::PLAYER_PROJECTILE_BLOCKING)
	{
		if (auto dome = hit.GameObject->GetComponent<BossDome>())
		{
			dome->OnHit(hit, myGameObject);
		}
	}
}

void PiercingComponent::Init()
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

	auto* audio = myGameObject->AddComponent<Engine::AudioComponent>();
	// TODO: Fix Sound
	audio->AddEvent("ChargeAttack", "event:/SFX/PLR/ChargeAttack");
	audio->PlayEvent("ChargeAttack");
}

void PiercingComponent::ScanDamage(float aDistance)
{
	std::vector<RayCastHit> hits;
	if (myGameObject->GetScene()->RayCastAll(GetTransform().GetPosition(), myDirection, aDistance, eLayer::PLAYER_PROJECTILE_HIT, hits))
	{
		for (int i = 0; i < hits.size(); i++)
		{
			OnHit(hits[i]);
		}
	}
}

void PiercingComponent::Explode()
{
	auto* scene = GetGameObject()->GetScene();

	if (!myExplosionPrefab || !myExplosionPrefab->IsValid())
	{
		LOG_ERROR(LogType::Game) << "Secondary ability missing explosion prefab!";
		return;
	}

	auto* obj = &myExplosionPrefab->Get().Instantiate(*scene);
	obj->GetTransform().SetPosition(myGameObject->GetTransform().GetPosition());

	auto* explosion = obj->AddComponent<Explosion>();

	ExplosionInfo explosionInfo;
	explosionInfo.explosionTime = myExplosionTime;
	explosionInfo.range = myExplosionRange;
	explosionInfo.knockback = myExplosionKnockback;
	explosionInfo.sizeCurve = myExplosionSizeCurve;
	explosionInfo.targetMask = eLayer::NONPLAYERDAMAGEABLE;

	DamageInfo dmgInfo = myDamageInfo;
	dmgInfo.damage = dmgInfo.damage * myExplosionDamageMultiplier;
	explosion->Init(explosionInfo, dmgInfo);


	auto* audio = obj->AddComponent<Engine::AudioComponent>();
	// TODO: Fix Sound
	audio->AddEvent("Explode", "event:/SFX/NPC/ProjectileExplode");
	audio->PlayEvent("Explode");

	return;

	//const float dmg = myDamageInfo.GetDamage() * myExplosionDamageMultiplier;

	//auto* player = reinterpret_cast<Player*>(myGameObject->GetSingletonComponent<PollingStationComponent>()->GetPlayerComponent());
	//std::vector<OverlapHit> hits;
	//auto scene = myGameObject->GetScene();
	//if (scene->SphereCastAll(GetTransform().GetPosition(), myExplosionRange, eLayer::NONPLAYERDAMAGEABLE, hits))
	//{
	//	for (int i = 0; i < hits.size(); i++)
	//	{
	//		auto& hit = hits[i];

	//		if (auto* hp = hit.GameObject->GetComponent<HealthComponent>()) 
	//		{
	//			hp->ApplyDamage(dmg, false, nullptr, myDamageInfo.IsCrit() ? eDamageType::Crit : eDamageType::Basic);
	//			if (player)
	//				player->GetCrosshair().HitMark();

	//			const auto dir = GetTransform().GetPosition() - hit.GameObject->GetTransform().GetPosition();
	//			hp->ApplyKnockback(dir.GetNormalized() * myExplosionKnockback);
	//		}
	//	}
	//}

	//if (!myExplosionVFX)
	//	return;
	//auto obj = GetGameObject()->GetScene()->AddGameObject<GameObject>();
	//obj->GetTransform().SetPosition(GetTransform().GetPosition());
	//obj->GetTransform().SetRotation(GetTransform().GetRotation());
	//auto vfx = obj->AddComponent<Engine::VFXComponent>(myExplosionVFX->Get());
	//vfx->Play();
	//vfx->AutoDestroy();

	//GDebugDrawer->DrawSphere3D(DebugDrawFlags::Always, GetTransform().GetPosition(), myExplosionRange, 1.f, Vec4f(1.f, 0, 0, .7f));
}

void PiercingComponent::KillInstance()
{
	if (!myVFXObj.expired())
	{
		myVFX->Stop();
	}
	myGameObject->Destroy();
}
 