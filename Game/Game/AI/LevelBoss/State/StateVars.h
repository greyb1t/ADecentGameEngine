#pragma once

#include "Engine/Reflection/Reflector.h"
//#include "Engine/GameObject/GameObjectRef.h"

BETTER_ENUM(LevelBossState, int,
	Unengaged = 0,
	Idle,
	Death,
	Disengaged,

	SpikeAttack,

	GatlingEye,
	Mortar,
	PoisonCloud

);

namespace LevelBossStates
{
	struct RoamVars : public Engine::Reflectable
	{
		float myIdleDurationMin = 2.0f;
		float myIdleDurationMax = 4.0f;

		float myIdleProgress = 0.0f;
		float myIdleDuration = 0.0f;

		void Reflect(Engine::Reflector& aReflector) override;
	};

	struct DisengageVars : public Engine::Reflectable
	{
		float myWaitUntilHealDuration = 2.0f;
		float myWaitUntilHealProgress = 0.0f;

		float myHealingAmountPerTick = 10.0f;
		float myHealingDuration = 0.1f;
		float myHealingProgress = 0.0f;

		VFXRef myHealingVFXRef;

		void Reflect(Engine::Reflector& aReflector) override;
	};

	struct AttackVars : public Engine::Reflectable
	{
		float myCooldownDurationMin = 2.0f;
		float myCooldownDurationMax = 4.0f;
		float myCooldownDuration = 4.0f;
		float myCooldownProgress = 2.0f;

		float myTelegraphDuration = 2.0f;
		float myTelegraphProgress = 0.0f;

		float myOverrideProgress = 0.0f;
		float myOverrideDuration = 2.0f;

		virtual void Reflect(Engine::Reflector& aReflector) override;
	};

	struct GatlingEyeVars : public AttackVars
	{
		float myFollowTargetDuration = 2.0f;
		float myFollowTargetProgress = 0.0f;

		float myTelegraphFollowTargetSpeed = 750.0f;
		float myFollowTargetSpeed = 750.0f;

		float myTimeBetweenShotsDuration = 0.1f;
		float myTimeBetweenShotsProgress = 0.0f;

		float mySpread = 150.f;
		float myMinDamage = 10.0f;
		float myMaxDamage = 20.0f;

		GameObjectPrefabRef myTelegraphingOrbPrefab;
		GameObjectPrefabRef myGatlingLaserPrefab;

		VFXRef myGatlingChargeVFXRef;
		VFXRef myGatlingImpactVFXRef;

		void Reflect(Engine::Reflector& aReflector) override;
	};

	struct PoisonCloudVars : public AttackVars
	{
		int myWidth = 3;
		float mySize = 250.0f;
		float myDistance = 5000.0f;

		float mySpeed = 2.0f;

		float myAttackProgress = 0.f;
		float myAttackDuration = 4.f;

		float myMinDamage = 1.0f;
		float myMaxDamage = 2.0f;

		float myTickRate = 0.2f;

		VFXRef myPoisonCloudVFXRef;

		void Reflect(Engine::Reflector& aReflector) override;
	};

	struct MortarVars : public AttackVars
	{
		float myMinDamage = 30.0f;
		float myMaxDamage = 40.0f;

		int myClusterCount = 20;
		float myClusterRadius = 200.0f;
		float myClusterOffsetSpread = 500.0f;
		float myClusterOffsetLanding = 0.1f;

		float myTimeUntilLand = 5.0f;

		GameObjectPrefabRef myMortarPrefab;

		void Reflect(Engine::Reflector& aReflector) override;
	};

	struct SpikeAttackVars : public AttackVars
	{
		float myFollowTargetDuration = 2.0f;
		float myFollowTargetProgress = 0.0f;

		float myLingeringAtTargetDuration = 2.0f;
		float myLingeringAtTargetProgress = 0.0f;

		float myTimeBetweenSpikesDuration = 0.5f;
		float myTimeBetweenSpikesProgress = 0.0f;

		float mySpikeTelegraphingDuration = 2.5f;

		float myMinDamage = 30.0f;
		float myMaxDamage = 40.0f;
		float mySpikeRadius = 200.0f;
		float myForce = 100.0f;

		bool myIsActive = false;

		GameObjectPrefabRef mySpikeMeshPrefab;
		GameObjectPrefabRef mySpikeTelegraphPrefab;
		std::vector<Engine::GameObjectRef> myBossSpikes;

		void Reflect(Engine::Reflector& aReflector) override;
	};
}
namespace LBS = LevelBossStates;