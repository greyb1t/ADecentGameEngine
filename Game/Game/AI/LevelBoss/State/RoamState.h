#pragma once

#include "EngageState.h"

namespace LevelBossStates
{
	class RoamState : public LBS::EngageState
	{
	public:
		RoamState() = delete;
		RoamState(LevelBoss& aBoss, RoamVars& someVariables);

		void Update() override;
		void OnEnterState() override;

	private:
		bool IsValid(const LevelBossState& aState);
		LevelBossState GetRandomAttack();

	private:
		RoamVars& myVars;

	};
}
