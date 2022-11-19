#pragma once

#include "AI/FinalBoss/States/FinalBossState.h"
#include "Utils/TickTimer.h"
#include <stack>

namespace Engine
{
	class Curve;
}

namespace FB
{
	class HandCrawler;
	struct HandCrawlerDesc;

	class HandCrawler_Loop : public BaseFinalBossState
	{
	public:
		enum class State
		{
			HuntPlayer,
			JumpTowardsPlayer,
			ReturnToStart,
		};

		HandCrawler_Loop(HandCrawler& aParent);

		void Update() override;

		bool IsFinished() override;

		void OnEnter() override;
		void OnExit() override;

	private:
		void StickToGround();

		Vec3f CreateZigZagPointTowardsPosition(const Vec3f& aTargetPos);
		void MoveTowardsNextPos();
		bool ReachedNextPos() const;
		float DistSq2D(const Vec3f& aFirst, const Vec3f& aSecond) const;

		void SetNextTargetPos(const Vec3f& aPos);

		void CheckPlayerCollision();
		void CrawlingSound(const bool aEnable);

	private:
		HandCrawler& myCrawler;

		TickTimer myTimer;
		bool myIsFinished = false;

		State myState = State::HuntPlayer;

		Vec3f myNextTargetPos;
		// bool myStartedAttackingPlayer = false;
		Vec3f mySampledPlayerPos;

		int myAttempts = 0;
		int myMaxAttempts = 0;

		// TickTimer myJumpTimer;
		float myJumpSpeedCurveMax = 0.f;
		float myJumpElapsedTime = 0.f;
		Vec3f myJumpStartPos;
		Vec3f myJumpTargetPos;

		Engine::Curve* myJumpSpeedCurve = nullptr;

		// float myStartRotY = 0.f;
		// float myTargetRotY = 0.f;
		// float myCurrentRotationY = 0.f;

		bool myHasHitPlayer = false;
	};
}