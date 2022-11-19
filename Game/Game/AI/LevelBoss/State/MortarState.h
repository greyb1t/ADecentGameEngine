#pragma once

#include "EngageState.h"

namespace LevelBossStates
{
	class MortarState : public LBS::EngageState
	{
	public:
		MortarState() = delete;
		MortarState(LevelBoss& aBoss, MortarVars& someVariables);

		void ForceUpdate() override;

		void OnEnterState() override;
		void OnExitState() override;

		void OnEnableAttack() override;
		void OnDisableAttack() override;

	private:
		void OnUpdate(const Vec3f& aPlayerPosition);

		void MortarAttackFrame();

	private:
		MortarVars& myVars;

		Vec3f myEyePosition = Vec3f(0.0f, 750.0f, 0.0f);

		enum class AnimState
		{
			Opening,
			Closing,
			None
		} myAnimState = AnimState::None;
	};
}
