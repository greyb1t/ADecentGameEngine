#pragma once

#include "Behaviour.h"
#include "AI/FinalBoss/States/FinalBossState.h"
#include "Utils/TickTimer.h"

namespace Engine
{
	class AnimatorComponent;
}

namespace FB
{
	class FinalBossHand;

	class HandEnraged : public Behaviour
	{
	public:
		enum class State
		{
			IsMovingToYLevel,
			BangGround
		};

		HandEnraged(FinalBossHand& aHand, Engine::AnimatorComponent& aAnimator);

		void Update() override;

		bool IsFinished() const override;

		void OnEnter() override;
		void OnExit() override;

		void Reflect(Engine::Reflector& aReflector) override;

		bool IsAllowedToMoveExternally() const override;

	private:
		FinalBossHand& myHand;
		Engine::AnimatorComponent& myAnimator;

		bool myIsFinished = false;

		State myState = State::IsMovingToYLevel;

		TickTimer myMoveToYLevelTimer;
		Vec3f myStartPos;
		Vec3f myTargetPos;
		Vec3f myHitGroundPos;
	};
}