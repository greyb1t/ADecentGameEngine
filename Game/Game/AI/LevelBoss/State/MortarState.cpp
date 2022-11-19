#include "pch.h"
#include "MortarState.h"

#include "Engine/GameObject/Prefab/GameObjectPrefab.h"
#include "../Objects/BossMortar.h"

LevelBossStates::MortarState::MortarState(LevelBoss& aBoss, MortarVars& someVariables) : EngageState(aBoss), myVars(someVariables)
{
	GetController().AddEventCallback("MortarAttackFrame", [this]() { MortarAttackFrame(); });

	OnExitState();

	myAnimState = AnimState::None;
}

void LevelBossStates::MortarState::ForceUpdate()
{
	myVars.myCooldownProgress -= Time::DeltaTime;
	if (myVars.myCooldownProgress <= 0.0f)
	{
		myVars.myCooldownProgress = 0.0f;
	}

	myVars.myOverrideProgress -= Time::DeltaTime;
	if (myAnimState == AnimState::Opening)
	{
		if (myVars.myOverrideProgress <= 0.0f)
		{
			myVars.myOverrideProgress = 0.0f;
			myAnimState = AnimState::None;
		}
		float ratio = myVars.myOverrideProgress / myVars.myOverrideDuration;
		GetController().SetLayerWeight("Mortar + Poison Is Disabled - (Override)", ratio);
	}
	else if (myAnimState == AnimState::Closing)
	{
		if (myVars.myOverrideProgress <= 0.0f)
		{
			myVars.myOverrideProgress = 0.0f;
			myAnimState = AnimState::None;
		}
		float ratio = 1.0f - (myVars.myOverrideProgress / myVars.myOverrideDuration);
		GetController().SetLayerWeight("Mortar + Poison Is Disabled - (Override)", ratio);
	}
}

void LevelBossStates::MortarState::OnEnterState()
{
	GetController().Trigger("TriggerMortar");
	GetAudio().PlayEvent("Mortar");
}

void LevelBossStates::MortarState::OnExitState()
{
	myVars.myCooldownDuration = Random::RandomFloat(myVars.myCooldownDurationMin, myVars.myCooldownDurationMax);
	myVars.myCooldownProgress = myVars.myCooldownDuration;

	myState = State::Idle;
}

void LevelBossStates::MortarState::OnEnableAttack()
{
	if (!myBoss.IsAbilityUnlocked(LevelBossState::PoisonCloud) && !myBoss.IsAbilityUnlocked(LevelBossState::Mortar))
	{
		myAnimState = AnimState::Opening;
		myVars.myOverrideProgress = myVars.myOverrideDuration;
	}
}

void LevelBossStates::MortarState::OnDisableAttack()
{
	if (!myBoss.IsAbilityUnlocked(LevelBossState::PoisonCloud) && !myBoss.IsAbilityUnlocked(LevelBossState::Mortar))
	{
		myAnimState = AnimState::Closing;
		myVars.myOverrideProgress = myVars.myOverrideDuration;
	}
}

void LevelBossStates::MortarState::OnUpdate(const Vec3f& aPlayerPosition)
{
	TurnTowards(aPlayerPosition);
}

void LevelBossStates::MortarState::MortarAttackFrame()
{
	myBoss.ChangeState(LevelBossState::Idle);

	if (!myVars.myMortarPrefab)
	{
		LOG_ERROR(LogType::Game) << "Missing mortar prefab. Did you forget to assign it?";
		return;
	}

	Engine::GameObjectPrefab& mortarPrefab = myVars.myMortarPrefab->Get();
	auto scene = myBoss.GetGameObject()->GetScene();
	GameObject& mortar = mortarPrefab.Instantiate(*scene);

	if (auto script = mortar.GetComponent<BossMortar>())
	{
		auto boneIndex = GetController().GetBoneIndexFromName("Tongue2");
		if (boneIndex < 0)
		{
			LOG_ERROR(LogType::Game) << "Tongue2 was not found on level boss skeleton";
			return;
		}

		auto boneTransform = GetController().GetBoneTransformWorld(boneIndex);

		Quatf dec_rot;
		Vec3f dec_scale;

		boneTransform.Decompose(myEyePosition, dec_rot, dec_scale);

		script->SetPositions(myEyePosition, 2500.f, myVars.myTimeUntilLand);

		script->SetDamage(Random::RandomFloat(myVars.myMinDamage, myVars.myMaxDamage) * myBoss.GetDmgMultiplier());
		script->SetClusterCount(myVars.myClusterCount);
		script->SetClusterRadius(myVars.myClusterRadius);
		script->SetOffsetLanding(myVars.myClusterOffsetLanding);
		script->SetOffsetSpread(myVars.myClusterOffsetSpread);
	}
}
