#pragma once

#include "AI/FinalBoss/States/FinalBossState.h"
#include "Utils/TickTimer.h"
#include "Common/Math/BezierSpline.h"

namespace Engine
{
	class Curve;
}

namespace FB
{
	class HandSlam;

	class HandSlam_Fall : public BaseFinalBossState
	{
	public:
		HandSlam_Fall(HandSlam& aParent);

		void Update() override;

		bool IsFinished() override;

		void OnEnter() override;
		void OnExit() override;

	private:
		void OnHitGround();
		void CheckPlayerCollision();

	private:
		HandSlam& myHandSlam;
		TickTimer myTimer;
		BezierSpline myBezier;
		bool myIsFinished = false;

		bool myHitGround = false;
		bool myHasHitPlayer = false;
	};
}