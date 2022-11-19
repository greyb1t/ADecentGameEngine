#pragma once

#include "Engine/GameObject/Components/Component.h"
#include "Engine/ResourceManagement/ResourceRef.h"
#include "Behaviours/HandCrawler.h"
#include "FinalBossHand.h"
#include "FinalBossDamage.h"

namespace FB
{
	BETTER_ENUM(RightHandBehaviours, int,
		Idle,
		Death,
		HomingLaser,
		GatlingEye,
		Crawler,
		Enraged,
		Intro);

	class Behaviour;

	struct HomingLaserDesc : public Engine::Reflectable
	{
		GameObjectPrefabRef myHomingLaserPrefab;

		VFXRef myBuildupVFX;

		FinalBossDamage myProjectileDamage;

		int myProjectileCount = 3;

		float mySphereOffsetLength = 1500.f;

		float mySecondsUntilSelfDestroy = 15.f;

		float myProjectileSpeed = 3000.f;

		void Reflect(Engine::Reflector& aReflector) override
		{
			aReflector.Reflect(myHomingLaserPrefab, "Homing Laser Prefab");
			aReflector.Reflect(myBuildupVFX, "Buildup VFX");
			aReflector.Reflect(myProjectileDamage, "Projectile Damage");
			aReflector.Reflect(myProjectileCount, "Projectile Count");
			aReflector.Reflect(mySphereOffsetLength, "Projectile Sphere Formation Size");
			aReflector.Reflect(myProjectileSpeed, "Projectile Speed");
		}
	};

	struct GattlingEyeDesc : public Engine::Reflectable
	{
		Vec2f myShootingDurationRange = Vec2f(5.f, 7.f);

		float mySpreadRadius = 150.f;

		float myRapidFireSpeedFactor = 1.f;

		float myTargetPlayerSpeed = 2.f;

		GameObjectPrefabRef myLaserPrefab;
		GameObjectPrefabRef myTelegraphOrbPrefab;

		float mySingleLaserDuration = 0.5f;
		FinalBossDamage myProjectileDamage;

		void Reflect(Engine::Reflector& aReflector) override
		{
			aReflector.Reflect(myShootingDurationRange, "Shooting Duration Range");
			aReflector.Reflect(mySpreadRadius, "Spread Radius");
			aReflector.Reflect(myRapidFireSpeedFactor, "Rapid Fire Speed");
			aReflector.Reflect(myLaserPrefab, "Laser Prefab");
			aReflector.Reflect(myTargetPlayerSpeed, "Follow Player Speed");
			aReflector.Reflect(myTelegraphOrbPrefab, "Telegraph Orb");
			aReflector.Reflect(mySingleLaserDuration, "Single Laser Duration");
			aReflector.Reflect(myProjectileDamage, "Damage");
		}
	};

	class RightProjectileHand : public FinalBossHand
	{
	public:
		COMPONENT(RightProjectileHand, "FB Right Projectile Hand");

		RightProjectileHand();
		RightProjectileHand(GameObject* aGameObject);
		virtual ~RightProjectileHand();

		const HomingLaserDesc& GetHomingLaserDesc() const;
		const GattlingEyeDesc& GetGattlingEyeDesc() const;

		bool IsDemolished() const override;

		void OnEnterPhase(const FinalBossPhase aPhase) override;

		float GetMaxAllowedDistanceAbovePortal() override;

	protected:
		void Awake() override;
		void Start() override;

		void Execute(Engine::eEngineOrder aOrder) override;

		void Reflect(Engine::Reflector& aReflector) override;

	private:
		void OnDeath() override;

	private:
		HomingLaserDesc myHomingLaserDesc;
		GattlingEyeDesc myGattlingEyeDesc;

		Engine::GameObjectRef myFinalBossGameObject;
		FinalBoss* myFinalBoss = nullptr;
	};
}