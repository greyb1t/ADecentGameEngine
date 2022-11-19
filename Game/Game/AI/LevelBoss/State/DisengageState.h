#pragma once

#include "EngageState.h"

namespace LevelBossStates
{
	class DisengageState : public LBS::EngageState
	{
	public:
		DisengageState() = delete;
		DisengageState(LevelBoss& aBoss, DisengageVars& someVariables);

		void Update() override;

		void OnEnterState() override;
		void OnExitState() override;

	private:
		void OnRapidHealing();

	private:
		DisengageVars& myVars;
		Engine::VFXComponent* myHealingVFX = nullptr;

	};
}

