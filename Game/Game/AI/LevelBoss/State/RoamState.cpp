#include "pch.h"
#include "RoamState.h"

LevelBossStates::RoamState::RoamState(LevelBoss& aBoss, RoamVars& someVariables) 
	: EngageState(aBoss), myVars(someVariables)
{
}

void LevelBossStates::RoamState::Update()
{
	auto pollingStation = GetPollingStation();
	if (!pollingStation)
		return;

	const LevelBossState state = GetRandomAttack();

	if (IsValid(state))
	{
		myBoss.ChangeState(state);
	}
	
	TurnTowards(pollingStation->GetPlayerPos());
}

void LevelBossStates::RoamState::OnEnterState()
{
	myVars.myIdleDuration = Random::RandomFloat(myVars.myIdleDurationMin, myVars.myIdleDurationMax);
	myVars.myIdleProgress = myVars.myIdleDuration;
}

bool LevelBossStates::RoamState::IsValid(const LevelBossState& aState)
{
	return myBoss.GetCooldownNormalized(aState) <= 0.0f;
}

LevelBossState LevelBossStates::RoamState::GetRandomAttack()
{
	const int startIndex = LevelBossState::GatlingEye;
	const int endIndex = startIndex + (int)LevelBossState::_size() - startIndex - 1;
	const int index = Random::RandomInt(startIndex, endIndex);
	LevelBossState state = LevelBossState::_from_integral(index);
	return state;
}
