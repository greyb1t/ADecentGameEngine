#pragma once

#include "Behaviour.h"
#include "AI/FinalBoss/States/FinalBossState.h"
#include "AI/FinalBoss/StateSequencer.h"

namespace FB
{
	struct HandSlamDesc;
	class LeftPunchHand;

	BETTER_ENUM(HandSlamStateType, int,
		Start,
		Search,
		Detect,
		Fall,
		End);

	class HandSlam : public Behaviour
	{
	public:
		HandSlam(LeftPunchHand& aLeftHand);

		void Update() override;

		bool IsFinished() const override;

		LeftPunchHand& GetLeftHand();

		const Vec3f& GetInitialPosition() const;

		const HandSlamDesc& Desc() const;

		void HandSlamSound(const bool aEnable) const;
		void HandSlamImpactSound(const bool aEnable) const;

	protected:
		void OnEnter();
		void OnExit();

		void Reflect(Engine::Reflector& aReflector) override;

		bool IsAllowedToMoveExternally() const override { return false; }

	private:
		LeftPunchHand& myLeftHand;

		Vec3f myInitialPosition;

		StateSequencer myStateSequencer;

		std::array<Owned<BaseFinalBossState>, HandSlamStateType::_size()> myStates;
	};
}