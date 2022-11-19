#pragma once

#include "EngageState.h"

namespace Engine
{
	class AnimatorComponent;
}

namespace LevelBossStates
{
	struct Spike
	{
		Vec3f myPosition = {};
		float myTelegraphProgress = 0.0f;
		float myTelegraphDuration = 0.0f;
		GameObject* myTelegraphObject = nullptr;
	};

	class SpikeAttackState : public LBS::EngageState
	{
	public:
		SpikeAttackState() = delete;
		SpikeAttackState(LevelBoss& aBoss, SpikeAttackVars& someVariables);

		void ForceUpdate() override;

		void OnEnableAttack() override;
		void OnDisableAttack() override;

	private:

		void EnterAttack();
		void Attack(const Vec3f& aPlayerPosition);
		void ExitAttack();

		void SpikeAttackSequence();

	private:
		SpikeAttackVars& myVars;
		std::vector<Spike> mySpikes;
		std::vector<Engine::AnimatorComponent*> myAnimators;

		enum class AnimState
		{
			Opening,
			Closing,
			None
		} myAnimState = AnimState::None;
	};
}
