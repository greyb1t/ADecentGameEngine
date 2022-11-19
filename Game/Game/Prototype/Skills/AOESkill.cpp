#include "pch.h"
#include "AOESkill.h"

#include "Engine/Shortcuts.h"
#include "Engine/GameObject/GameObject.h"
#include "Prototype/BouncingProjectile.h"
#include "Player/Player.h"

#include "HUD\HUDHandler.h"
#include "Game/Camera/CameraController.h"
#include "Player/DamageInfo.h"

AOESkill::AOESkill()
{
}

AOESkill::AOESkill(Player* aPlayer)
	: Skill(aPlayer)
{
}

void AOESkill::Activate()
{
	Shoot();
	Skill::Activate(); 
	Main::PlayerShootEvent();
}

void AOESkill::Shoot()
{
	myPlayer->GetHUDHandler()->StartCooldown(HUDHandler::AbilityType::Utiliy);

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
		Vec3f pos = cc.GetPivotPoint(); //myPlayer->GetHead()->GetTransform().GetPosition();
		if (scene->RayCast(
			pos, cameraDir, maxAimDistance,
			layerMask,
			hit, false))
		{
			aimPosition = hit.Position;
		}
	}

	const Vec3f dir = (aimPosition - muzzleOrigin).GetNormalized();

	auto* obj = myPlayer->GetGameObject()->GetScene()->AddGameObject<GameObject>();
	obj->GetTransform().SetPosition(muzzleOrigin);
	auto* projectile = obj->AddComponent<BouncingProjectile>();

	auto& stats = myPlayer->GetSkillStats().utility;

	const float damage = myPlayer->GetStats().GetDamage() * stats.damageMultiplier + stats.baseDamage;
	DamageInfo dmgInfo(damage, myPlayer->GetStats().GetCritChance(), myPlayer->GetStats().GetCritDamageMultiplier());

	auto& vfxs = myPlayer->GetVFXHolder().utility;

	projectile->Setup(dir, dmgInfo, stats.bounces, stats.bounceRange, stats.projectileSpeed, stats.bounceProjectileSpeed, stats.projectileRange, vfxs.projectile, vfxs.hit);
}

