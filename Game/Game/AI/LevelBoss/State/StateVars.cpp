#include "pch.h"
#include "StateVars.h"

void LevelBossStates::RoamVars::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myIdleDurationMin, "Idle Duration Min");
	aReflector.Reflect(myIdleDurationMax, "Idle Duration Max");
	aReflector.Reflect(myIdleDuration, "Idle Duration", Engine::ReflectionFlags::ReflectionFlags_ReadOnly);
	aReflector.Reflect(myIdleProgress, "Idle Progress", Engine::ReflectionFlags::ReflectionFlags_ReadOnly);
}

void LevelBossStates::AttackVars::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myCooldownDurationMin, "Cooldown Min");
	aReflector.Reflect(myCooldownDurationMax, "Cooldown Max");
	aReflector.Reflect(myCooldownDuration, "Cooldown Duration", Engine::ReflectionFlags::ReflectionFlags_ReadOnly);
	aReflector.Reflect(myCooldownProgress, "Cooldown Progress", Engine::ReflectionFlags::ReflectionFlags_ReadOnly);

	aReflector.Reflect(myTelegraphDuration, "Telegraph Duration");
	aReflector.Reflect(myTelegraphProgress, "Telegraph Progress", Engine::ReflectionFlags::ReflectionFlags_ReadOnly);

	aReflector.Reflect(myOverrideDuration, "Override Duration");
	aReflector.Reflect(myOverrideProgress, "Override Progress", Engine::ReflectionFlags::ReflectionFlags_ReadOnly);
}

void LevelBossStates::GatlingEyeVars::Reflect(Engine::Reflector& aReflector)
{
	AttackVars::Reflect(aReflector);
	
	aReflector.Reflect(myFollowTargetDuration, "Follow Duration");
	aReflector.Reflect(myFollowTargetProgress, "Follow Progress", Engine::ReflectionFlags::ReflectionFlags_ReadOnly);

	aReflector.Reflect(myTelegraphFollowTargetSpeed, "Telegraph Follow Speed");
	aReflector.Reflect(myFollowTargetSpeed, "Attack Follow Speed");

	aReflector.Reflect(mySpread, "Bullet Spread");

	aReflector.Reflect(myTimeBetweenShotsDuration, "Time Between Shots Duration");
	aReflector.Reflect(myTimeBetweenShotsProgress, "Time Between Shots Progress", Engine::ReflectionFlags::ReflectionFlags_ReadOnly);

	aReflector.Reflect(myMinDamage, "Min Damage");
	aReflector.Reflect(myMaxDamage, "Max Damage");

	aReflector.Reflect(myTelegraphingOrbPrefab, "Telegraphing Orb Prefab");
	aReflector.Reflect(myGatlingLaserPrefab, "Gatling Eye Laser");
	aReflector.Reflect(myGatlingChargeVFXRef, "Gatling Eye Charge");
	aReflector.Reflect(myGatlingImpactVFXRef, "Gatling Eye Impact");
}

void LevelBossStates::MortarVars::Reflect(Engine::Reflector& aReflector)
{
	AttackVars::Reflect(aReflector);

	aReflector.Reflect(myMinDamage, "Min Damage");
	aReflector.Reflect(myMaxDamage, "Max Damage");

	aReflector.Reflect(myClusterCount, "Cluster Amount");
	aReflector.Reflect(myClusterRadius, "Cluster Radius");
	aReflector.Reflect(myClusterOffsetSpread, "Cluster Spread");
	aReflector.Reflect(myClusterOffsetLanding, "Cluster Offset");

	aReflector.Reflect(myTimeUntilLand, "Time Until Land");

	aReflector.Reflect(myMortarPrefab, "Mortar Prefab");
}

void LevelBossStates::SpikeAttackVars::Reflect(Engine::Reflector& aReflector)
{
	AttackVars::Reflect(aReflector);

	aReflector.Reflect(mySpikeMeshPrefab, "Spike Mesh Prefab");
	aReflector.Reflect(myBossSpikes, "Boss Spikes");

	aReflector.Reflect(myFollowTargetDuration, "Follow Duration");
	aReflector.Reflect(myFollowTargetProgress, "Follow Progress", Engine::ReflectionFlags::ReflectionFlags_ReadOnly);

	aReflector.Reflect(myLingeringAtTargetDuration, "Stay Duration");
	aReflector.Reflect(myLingeringAtTargetProgress, "Stay Progress", Engine::ReflectionFlags::ReflectionFlags_ReadOnly);

	aReflector.Reflect(myTimeBetweenSpikesDuration, "Time Between Spikes Duration");
	aReflector.Reflect(myTimeBetweenSpikesProgress, "Time Between Spikes Progress", Engine::ReflectionFlags::ReflectionFlags_ReadOnly);
	
	aReflector.Reflect(mySpikeTelegraphingDuration, "Spike Telegraph Duration");
	aReflector.Reflect(mySpikeTelegraphPrefab, "Spike Telegraph Prefab");


	aReflector.Reflect(myMinDamage, "Min Damage");
	aReflector.Reflect(myMaxDamage, "Max Damage");
	aReflector.Reflect(myForce, "Force");
	aReflector.Reflect(mySpikeRadius, "Radius");
}

void LevelBossStates::PoisonCloudVars::Reflect(Engine::Reflector& aReflector)
{
	AttackVars::Reflect(aReflector);

	if (aReflector.Reflect(myDistance, "Distance") & Engine::ReflectorResult::ReflectorResult_Changing)
	{
		myDistance = CU::Abs(myDistance);
	}

	if (aReflector.Reflect(myWidth, "Width") & Engine::ReflectorResult::ReflectorResult_Changing)
	{
		if (myWidth < 1)
		{
			myWidth = 1;
		}
	}

	if (aReflector.Reflect(mySize, "Size") & Engine::ReflectorResult::ReflectorResult_Changing)
	{
		if (mySize < 50.f)
			mySize = 50.0f;
	}

	if (aReflector.Reflect(mySpeed, "Duration") & Engine::ReflectorResult::ReflectorResult_Changing)
	{
	}

	if (aReflector.Reflect(myAttackDuration, "Speed") & Engine::ReflectorResult::ReflectorResult_Changing)
	{
	}

	aReflector.Reflect(myMinDamage, "Min Damage");
	aReflector.Reflect(myMaxDamage, "Max Damage");
	aReflector.Reflect(myTickRate, "Tick Rate");

	aReflector.Reflect(myPoisonCloudVFXRef, "Poison Cloud VFX");
}

void LevelBossStates::DisengageVars::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myWaitUntilHealDuration, "Wait Until Heal Duration");
	aReflector.Reflect(myWaitUntilHealProgress, "Wait Until Heal Progress", Engine::ReflectionFlags::ReflectionFlags_ReadOnly);

	aReflector.Reflect(myHealingAmountPerTick, "Heal Amount");
	aReflector.Reflect(myHealingDuration, "Heal Tick Time");

	aReflector.Reflect(myHealingVFXRef, "Healing VFX");
}
