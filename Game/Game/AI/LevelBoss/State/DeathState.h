#pragma once

#include "BaseState.h"
#include "StateVars.h"

namespace Engine
{
	class AnimatorComponent;
}

namespace LevelBossStates
{
	class DeathState : public LBS::BaseState
	{
	public:
		DeathState() = delete;
		DeathState(LevelBoss& aBoss, SpikeAttackVars& someVariables);

		void OnEnterState() override;
		void OnExitState() override;

		void Update() override;

	private:
		SpikeAttackVars& myVars;
		std::vector<Engine::AnimatorComponent*> myAnimators;

		// if boss gets stuck, force it to die
		float myProgress = 0.0f;
		float myDuration = 20.0f;
		bool myEventWasCalled = false;

	};
}