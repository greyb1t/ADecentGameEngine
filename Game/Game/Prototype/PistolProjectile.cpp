#include "pch.h"
#include "PistolProjectile.h"

#include "DamageNumber.h"
#include "Components/HealthComponent.h"
#include "Engine/Shortcuts.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/GameObject/Components/AudioComponent.h"
#include "Engine/VFX/VFXUtilities.h"
#include "Game/AI/LevelBoss/Objects/BossDome.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"

#include "Items/ItemManager.h"
#include <AI/PollingStation/PollingStationComponent.h>
#include <Player/Player.h>

PistolProjectile::PistolProjectile(GameObject* aObject)
	: Component(aObject)
{
}

void PistolProjectile::Setup(const Vec3f& aDir, const DamageInfo& aDamageInfo, float aSpeed, float aRange, float aProjectileLength, VFXRef aHitVFX)
{
	myDirection = aDir;
	myDamageInfo = aDamageInfo;
	mySpeed = aSpeed;
	myRange = aRange;
	myTarget = myRange;
	myProjectileLength = aProjectileLength;

	myHitVFX = aHitVFX;

	auto obj = GetGameObject()->GetScene()->AddGameObject<GameObject>();
	myVFXObj = obj->GetWeak();
	obj->GetTransform().SetParent(&GetTransform());
	obj->GetTransform().SetPositionLocal({ 0,0,0 });
	//myVFX = obj->AddComponent<Engine::VFXComponent>(VFX::Load("Assets/VFX/PISTOL_PROJECTILE.vfx"));
	//myVFX->Play();
	//myVFX->AutoDestroy();

	GetTransform().SetRotation(Quatf(Mat4f::LookAt(Vec3f(0, 0, 0), aDir)) * Quatf(Vec3f(0,90.f * Math::Deg2Rad,0)));

	auto* audio = obj->AddComponent<Engine::AudioComponent>();
	audio->AddEvent("PrimaryHit", "event:/SFX/NPC/PlayerWeaponImpact");

	HitScan();
}

void PistolProjectile::Start()
{
}

void PistolProjectile::Execute(Engine::eEngineOrder aOrder)
{
	float distance = mySpeed * Time::DeltaTime;
	/*LayerMask mask = eLayer::PLAYER_PROJECTILE_HIT;

	RayCastHit hit;
	if (myGameObject->GetScene()->RayCast(GetTransform().GetPosition(), myDirection, distance, mask, hit))
	{
		if (!(hit.Layer & eLayer::PLAYER))
		{
			GetTransform().SetPosition(hit.Position);
			myGameObject->Destroy();
			OnHit(hit);
			return;
		}
	}*/
	if (myDistanceTraveled + distance >= myTarget)
	{

		distance = myTarget - myDistanceTraveled;
		if (distance > myProjectileLength * .5f)
		{
			distance -= myProjectileLength * .5f;
		}

		myGameObject->Destroy();
	}

	myDistanceTraveled += distance;

	const Vec3f movement = myDirection * distance;

	GetTransform().Move(movement);
}

void PistolProjectile::OnHit(RayCastHit& hit)
{
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
		if (hp)
		{
			hp->ApplyDamage(damage, false, &hit.Position, myDamageInfo.IsCrit() ? eDamageType::Crit : eDamageType::Basic);
			if (auto pollingstation = myGameObject->GetSingletonComponent<PollingStationComponent>()) 
			{
				auto* player = reinterpret_cast<Player*>(pollingstation->GetPlayerComponent());
				if (player)
					player->GetCrosshair().HitMark();
			}
		}
		
	}
	if (hit.Layer & eLayer::PLAYER_PROJECTILE_BLOCKING)
	{
		if (auto dome = hit.GameObject->GetComponent<BossDome>())
		{
			dome->OnHit(hit, myGameObject);
		}
	}

	//if (!myVFXObj.expired())
	//	myVFX->Stop();

	if (myHitVFX && myHitVFX->IsValid())
	{
		auto obj = GetGameObject()->GetScene()->AddGameObject<GameObject>();
		obj->GetTransform().SetPosition(hit.Position - myDirection * 25.f);
		auto vfx = obj->AddComponent<Engine::VFXComponent>(myHitVFX->Get());
		vfx->Play();
		vfx->AutoDestroy();

		auto* audio = obj->AddComponent<Engine::AudioComponent>();
		// TODO: Fix
		audio->PlayEvent("PrimaryHit");
	}
}

void PistolProjectile::HitScan()
{
	LayerMask mask = eLayer::PLAYER_PROJECTILE_HIT;

	RayCastHit hit;
	if (myGameObject->GetScene()->RayCast(GetTransform().GetPosition(), myDirection, myRange, mask, hit))
	{
		myTarget = hit.Distance;
		OnHit(hit);
	}
}
