#pragma once

#include "Behaviour.h"
#include "AI/FinalBoss/StateSequencer.h"
#include "AI/FinalBoss/States/FinalBossState.h"

namespace FB
{
	class RightProjectileHand;

	BETTER_ENUM(HandGattlingEyeStates, int,
		Start,
		Loop,
		End);

	class HandGattlingEye : public Behaviour
	{
	public:
		HandGattlingEye(RightProjectileHand& aRightHand);

		void Update() override;

		bool IsFinished() const override;

		void OnEnter() override;
		void OnExit() override;

		void Reflect(Engine::Reflector& aReflector) override;

		RightProjectileHand& GetRightHand();

		bool IsAllowedToMoveExternally() const override;

	private:
		RightProjectileHand& myRightHand;

		StateSequencer myStateSequencer;

		std::array<Owned<BaseFinalBossState>, HandGattlingEyeStates::_size()> myStates;
	};
}