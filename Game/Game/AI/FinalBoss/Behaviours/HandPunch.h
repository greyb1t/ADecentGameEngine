#pragma once

#include "Behaviour.h"
#include "AI/FinalBoss/States/FinalBossState.h"
#include "AI/FinalBoss/StateSequencer.h"

namespace FB
{
	class LeftPunchHand;

	BETTER_ENUM(HandPunchStateType, int,
		Buildup,
		MoveTowardsPlayer,
		Returning);

	class HandPunch : public Behaviour
	{
	public:
		HandPunch(LeftPunchHand& aLeftHand);

		void Update() override;

		bool IsFinished() const override;

		LeftPunchHand& GetLeftHand();

		const Vec3f& GetInitialPosition() const;

		void HandPunchSound(const bool aEnable) const;

		bool HasHitPlayer() const;

	protected:
		void OnEnter();
		void OnExit();

		void Reflect(Engine::Reflector& aReflector) override;

		void OnCollisionEnter(GameObject& aGameObject) override;
		void OnCollisionStay(GameObject& aGameObject) override;

		bool IsAllowedToMoveExternally() const override { return false; }

	private:
		LeftPunchHand& myLeftHand;

		StateSequencer myStateSequencer;

		Vec3f myInitialPosition;

		bool myHasHitPlayer = false;

		std::array<Owned<BaseFinalBossState>, HandPunchStateType::_size()> myStates;
	};
}