#include "pch.h"
#include "PistolSkill.h"

#include "Components/HealthComponent.h"
#include "Engine/Shortcuts.h"
#include "Engine/GameObject/GameObject.h"
#include "Prototype/PistolProjectile.h"
#include "Items/ItemManager.h"
#include "Player/Player.h"

#include "Camera\CameraController.h"
#include <Prototype/HomingProjectile.h>
#include "HUD\HUDHandler.h"

#include "Player/DamageInfo.h"

#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/GameObject/Components/AudioComponent.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/ResourceManagement/Resources/GameObjectPrefabResource.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"

PistolSkill::PistolSkill(Player* aPlayer)
	: Skill(aPlayer)
{
}

void PistolSkill::Activate()
{
	Shoot();
	// TODO: Better solution where all skills update if new item or effect is added
	myCooldown = 1.f / (myPlayer->GetStats().GetAttackSpeed() * myPlayer->GetSkillStats().primary.attackSpeedMultiplier);
	myPlayer->GetMagicCircle().Primary();
	Skill::Activate();
	Main::PlayerShootEvent();
}

void PistolSkill::Shoot()
{
	Main::GetItemManager().PostEvent(eItemEvent::OnShoot, nullptr);
	myPlayer->GetGameObject()->GetComponent<Engine::AudioComponent>()->PlayEvent("PrimaryAttack");


	//myPlayer->GetHUDHandler()->StartCooldown(HUDHandler::AbilityType::PrimaryFire);

	Transform& transform = myPlayer->GetGameObject()->GetTransform();
	// Shoot position
	const Vec3f muzzleOrigin = myPlayer->GetMagicCircle().GetMuzzlePosition();

	auto* scene = myPlayer->GetGameObject()->GetScene();

	const float maxAimDistance = 100 * 100;

	auto& cc = myPlayer->GetCameraController();
	Transform& cameraTransform = cc.GetCamera();
	const Vec3f cameraDir = cameraTransform.Forward();
	Vec3f aimPosition = cameraTransform.GetPosition() + cameraDir * maxAimDistance;
	const LayerMask layerMask = eLayer::DEFAULT | eLayer::NONPLAYERDAMAGEABLE | eLayer::GROUND;
	{
		// Find what we are aiming at
		RayCastHit hit;
		Vec3f pos = cc.GetPivotPoint();
		if (scene->RayCast(
			pos, cameraDir, maxAimDistance,
			layerMask,
			hit, false))
		{
			aimPosition = hit.Position;
		}
	}
	auto& stats = myPlayer->GetSkillStats().primary;

	auto& steadiness = myPlayer->GetSteadiness();

	CU::Vector3f shootingDir = (aimPosition - muzzleOrigin).GetNormalized();
	CU::Vector3f forwardNormal = shootingDir.Cross({ 0.f, 1.f, 0.f });
	forwardNormal.Normalize();

	forwardNormal = forwardNormal * CU::Quaternion::AxisAngle(shootingDir, Random::RandomFloat(0, 360 * Math::Deg2Rad));

	const float steadinessSpread = steadiness.Get() * stats.steadinessSpread * .5f;

	const float r = Random::RandomFloat(-1, 1);
	const float weight = pow(r, stats.steadinessWeightPower) * (r > 0 ? 1 : -1);
	const float angle = weight * steadinessSpread;

	Vec3f dir = shootingDir * CU::Quaternion::AxisAngle(forwardNormal,
		angle * Math::Deg2Rad);
	dir.Normalize();

	const Vec3f firePosition = myPlayer->GetMagicCircle().GetMuzzlePosition() + dir * myPlayer->GetSkillStats().primary.projectileLength * 0.5f;

	auto projPrefab = myPlayer->GetVFXHolder().primary.projectile;

	if (!projPrefab || !projPrefab->IsValid())
	{
		LOG_WARNING(LogType::Game) << "Player missing projectile prefab reference!";
		return;
	}

	auto* obj  = &projPrefab->Get().Instantiate(*scene);
	obj->GetTransform().SetPosition(firePosition);

	auto* projectile = obj->AddComponent<PistolProjectile>();
	const float damage = myPlayer->GetStats().GetDamage() * stats.damageMultiplier * Random::RandomFloat(0.88f, 1.12f);
	const DamageInfo damageInfo(damage, myPlayer->GetStats().CritChance(), myPlayer->GetStats().GetCritDamageMultiplier());

	projectile->Setup(dir, damageInfo, stats.projectileSpeed, stats.projectileRange, myPlayer->GetSkillStats().primary.projectileLength, myPlayer->GetVFXHolder().primary.hit);

	//}

	steadiness.Add(myPlayer->GetSkillStats().primary.steadinessShootLoss);

	myPlayer->GetMagicCircle().GetMuzzleFlash()->Play();
	//VFXRef fireVFX = myPlayer->GetVFXHolder().primary.fire;
	/*if (fireVFX && fireVFX->IsValid())
	{
		auto obj = myPlayer->GetGameObject()->GetScene()->AddGameObject<GameObject>();
		obj->GetTransform().SetPosition(myPlayer->GetTransform().GetPosition());
		obj->GetTransform().SetRotation(Quatf(Mat4f::LookAt({0,0,0}, dir)));
		auto vfx = obj->AddComponent<Engine::VFXComponent>(fireVFX->Get());
		vfx->Play();
		vfx->AutoDestroy();
	}*/
}