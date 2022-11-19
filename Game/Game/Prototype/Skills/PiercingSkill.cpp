#include "pch.h"
#include "PiercingSkill.h"

#include "Engine/Shortcuts.h"
#include "Engine/GameObject/GameObject.h"
#include "Player/Player.h"

#include "Game/Camera/CameraController.h"
#include "Prototype/PiercingComponent.h"
#include "HUD\HUDHandler.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/ResourceManagement/Resources/GameObjectPrefabResource.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/GameObject/Components/AudioComponent.h"

PiercingSkill::PiercingSkill(Player* aPlayer)
	: Skill(aPlayer)
{

}

void PiercingSkill::Activate()
{
	myIsActive = true;
	myCharge = 0;
	Main::PlayerShootEvent();
	myPlayer->GetAudioComponent().PlayEvent("SecondaryCharge");
}

void PiercingSkill::Update()
{
	if (myIsActive)
		Charge();

	Skill::Update();
}

void PiercingSkill::Interrupt()
{
	myIsActive = false;
	myPlayer->GetLeftArm().DisableArm();
	myPlayer->GetAudioComponent().StopEvent("SecondaryCharge");
}

void PiercingSkill::Shoot()
{
	myPlayer->GetHUDHandler()->StartCooldown(HUDHandler::AbilityType::SecondaryFire);
	myPlayer->GetAudioComponent().StopEvent("SecondaryCharge");

	ShootProjectile();
	myIsActive = false;
	myCharge = 0;
	
	Skill::Activate();
}

void PiercingSkill::Charge()
{
	myCharge += Time::DeltaTime / myPlayer->GetSkillStats().secondary.chargeTime;
	myCharge = std::min(myCharge, 1.f);

	Transform& transform = myPlayer->GetGameObject()->GetTransform();

	GDebugDrawer->DrawSphere3D(DebugDrawFlags::Always, myPlayer->GetTransform().GetPosition(), 10 + 30 * myCharge, 0, Vec4f(1, 0.f, 0.f, 1.f));

}

void PiercingSkill::ShootProjectile()
{
	Transform& transform = myPlayer->GetLeftArm().GetForearm()->GetTransform();
	// Shoot position
	const Vec3f muzzleOrigin = transform.GetPosition();

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

	const Vec3f dir = (aimPosition - muzzleOrigin).GetNormalized();

	//GDebugDrawer->DrawLine3D(DebugDrawFlags::Always, myPlayer->GetTransform().GetPosition(), myPlayer->GetTransform().GetPosition() + dir * 1000.f, 2, Vec4f(0, 0, 1.f, .7f));
	//GDebugDrawer->DrawSphere3D(DebugDrawFlags::Always, myPlayer->GetTransform().GetPosition(), 30, 2, Vec4f(1, 0.f, 0.f, 1.f));

	auto projPrefab = myPlayer->GetVFXHolder().secondary.projectilePrefab;

	if (!projPrefab || !projPrefab->IsValid())
	{
		LOG_WARNING(LogType::Game) << "Player missing SECONDARY projectile prefab reference!";
		return;
	}

	auto* obj = &projPrefab->Get().Instantiate(*scene);
	obj->GetTransform().SetPosition(muzzleOrigin);
	auto* projectile = obj->AddComponent<PiercingComponent>();

	auto& stats = myPlayer->GetSkillStats().secondary;

	float damage = (stats.baseDamage + myPlayer->GetStats().GetDamage() * stats.playerDamageMultiplier) * (stats.minDamageMultiplier + ((stats.maxDamageMultiplier - stats.minDamageMultiplier) * myCharge));

	float projSpeed = stats.minProjectileSpeed + ((stats.maxProjectileSpeed - stats.minProjectileSpeed) * myCharge);

	float explosionRange = stats.explosionMinRange + ((stats.explosionMaxRange - stats.explosionMinRange) * myCharge);

	auto& vfxs = myPlayer->GetVFXHolder().secondary;

	DamageInfo dmgInfo(damage, myPlayer->GetStats().GetCritChance(), myPlayer->GetStats().GetCritDamageMultiplier());
	projectile->Setup(dir, dmgInfo, stats.damageRadius, projSpeed, stats.projectileRange, explosionRange, stats.explosionDamageMultiplier, vfxs.projectile, vfxs.hit, vfxs.explosion, vfxs.explosionPrefab, vfxs.explosionScaling, stats.explosionTime);

	VFXRef muzzleFlashVFX = myPlayer->GetVFXHolder().secondary.muzzleFlash;
	if (muzzleFlashVFX && muzzleFlashVFX->IsValid())
	{
		auto obj = myPlayer->GetGameObject()->GetScene()->AddGameObject<GameObject>();
		obj->GetTransform().SetPosition(muzzleOrigin);
		obj->GetTransform().SetRotation(Quatf(Mat4f::LookAt({0,0,0}, dir)));
		auto vfx = obj->AddComponent<Engine::VFXComponent>(muzzleFlashVFX->Get());
		vfx->Play();
		vfx->AutoDestroy();
	}
	myPlayer->GetAudioComponent().PlayEvent("SecondaryAttack");
}
