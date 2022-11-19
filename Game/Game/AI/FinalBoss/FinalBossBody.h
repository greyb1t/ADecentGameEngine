#pragma once

#include "Engine/GameObject/Components/Component.h"
#include "Utils/TickTimer.h"
#include "AI/FinalBoss/FinalBossPhase.h"
#include "FinalBossHand.h"
#include "FinalBossDamage.h"

namespace FB
{
	class FinalBoss;
	class Behaviour;

	BETTER_ENUM(FinalBossBodyBehaviours, int,
		Idle,
		Death,
		GigaLaser,
		MouthMortar,
		ShockwaveScream,
		PoisonBreath,
		Intro,
		PhaseSwitch);

	struct ShockwaveDesc : public Engine::Reflectable
	{
		GameObjectPrefabRef myShockwavePrefab;

		int myTotalShockwaves = 3;
		float myShockwaveDelaySec = 1.f;

		FinalBossDamage myDamage;

		float mySpeed = 100.f;
		float myKnockbackStrength = 100.f;
		float mySphereThiccness = 100.f;
		float myShockwaveHeight = 100.f;

		void Reflect(Engine::Reflector& aReflector) override
		{
			aReflector.Reflect(myShockwavePrefab, "Shockwave Prefab");
			aReflector.Reflect(myDamage, "Damage");

			aReflector.Reflect(mySpeed, "Shockwave Move Speed");
			aReflector.Reflect(myKnockbackStrength, "Knockback Strength");
			aReflector.Reflect(mySphereThiccness, "Shockwave Thiccness");
			aReflector.Reflect(myShockwaveHeight, "Shockwave Height");

			aReflector.Reflect(myShockwaveDelaySec, "Shockwave Delay Sec");
			aReflector.Reflect(myTotalShockwaves, "Total Shockwaves");
		}
	};

	struct GigaLaserDesc : public Engine::Reflectable
	{
		GameObjectPrefabRef myLaserPrefab;
		GameObjectPrefabRef myTelegraphLaserPrefab;

		FinalBossDamage myTotalDamage;

		int myTotalDamageTicks = 5;

		VFXRef myTelegraphMouthSuckVFX;
		VFXRef myTelegraphImpactVFX;
		VFXRef myGigaLaserImpactVFX;

		void Reflect(Engine::Reflector& aReflector) override
		{
			aReflector.Reflect(myLaserPrefab, "Laser Prefab");
			aReflector.Reflect(myTelegraphLaserPrefab, "Telegraph Laser Prefab");
			aReflector.Reflect(myTotalDamage, "Total Damage");
			aReflector.Reflect(myTotalDamageTicks, "Total Damage Ticks");
			aReflector.Reflect(myTelegraphImpactVFX, "Telegraph Impact VFX");
			aReflector.Reflect(myGigaLaserImpactVFX, "GigaLaser Impact VFX");
			aReflector.Reflect(myTelegraphMouthSuckVFX, "Telegraph Mouth Suck VFX");
		}
	};

	struct MortarDesc : public Engine::Reflectable
	{
		GameObjectPrefabRef myMortarPrefab;

		FinalBossDamage myDamage;

		int myClusterCount = 20;
		float myClusterRadius = 200.0f;
		float myClusterOffsetSpread = 500.0f;
		float myClusterOffsetLanding = 0.1f;

		float myTimeUntilLand = 5.0f;

		void Reflect(Engine::Reflector& aReflector) override
		{
			aReflector.Reflect(myMortarPrefab, "Mortar Prefab");
			aReflector.Reflect(myTimeUntilLand, "Time Until Land");

			aReflector.Reflect(myDamage, "Damage");

			aReflector.Reflect(myClusterCount, "Cluster Amount");
			aReflector.Reflect(myClusterRadius, "Cluster Radius");
			aReflector.Reflect(myClusterOffsetSpread, "Cluster Spread");
			aReflector.Reflect(myClusterOffsetLanding, "Cluster Offset");
		}
	};

	struct PoisonCloudDesc : public Engine::Reflectable
	{
		float myAngle = 30.f;
		float myLength = 10000.f;
		Engine::GameObjectRef mySpawnPoint;
		VFXRef myBreathParticle;
		float myDamageTicksPerSecond = 2.f;
		//float myTickDamage = 5.f;
		FinalBossDamage myTickDamage;
		bool myVisualizeHitbox = false;
		float myCloudsPerSecond = 5.f;
		VFXRef myCloudVFX;

		void Reflect(Engine::Reflector& aReflector) override
		{
			aReflector.Reflect(myAngle, "Angle");
			aReflector.Reflect(myLength, "Length");
			aReflector.Reflect(mySpawnPoint, "Spawn Point");
			aReflector.Reflect(myBreathParticle, "Poison Cloud");
			aReflector.Reflect(myDamageTicksPerSecond, "Damage Ticks Per Second");
			aReflector.Reflect(myTickDamage, "Damage Each Tick");
			aReflector.Reflect(myVisualizeHitbox, "Visualize Hitbox");
			aReflector.Reflect(myCloudsPerSecond, "Cloud Spawns Per Second");
			aReflector.Reflect(myCloudVFX, "Ground Cloud VFX");
		}
	};

	class FinalBossBody : public FinalBossPart
	{
	public:
		COMPONENT(FinalBossBody, "FB Body");

		FinalBossBody() = default;
		FinalBossBody(GameObject* aGameObject);

		void Awake() override;
		void Start() override;

		void InitAudio();

		void Reflect(Engine::Reflector& aReflector) override;
		void Execute(Engine::eEngineOrder aOrder);

		void EnterPhase(const FinalBossPhase aPhase);

		const GigaLaserDesc& GetGigaLaserDesc() const;
		const MortarDesc& GetMortarDesc() const;
		PoisonCloudDesc& GetPoisonCloudDesc();
		const ShockwaveDesc& GetShockwaveDesc() const;

		Engine::AudioComponent* GetAudio();
		Engine::AudioComponent* GetBodyAudio();

		bool IsIntroFinished() const override;

		bool IsDemolished() const override;

		void SetShouldRotateTowardsPlayer(const bool aValue);
		void SetIsIntroFinished(const bool aIsFinished);

	private:
		void OnDamaged(const float aDamage) override;
		void OnDeath() override;

		void RotateTowardsPlayer();

	private:
		GigaLaserDesc myGigaLaserDesc;
		MortarDesc myMortarDesc;
		PoisonCloudDesc myPoisonCloudDesc;
		ShockwaveDesc myShockwaveDesc;

		Engine::AudioComponent* myAudio = nullptr;

		bool myShouldRotateTowardsPlayer = true;

		Quatf myCurrentRotation;

		bool myIsIntroFinished = false;
	};
}