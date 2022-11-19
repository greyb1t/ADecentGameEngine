#pragma once

#include "AI/FinalBoss/States/FinalBossState.h"
#include "Utils/TickTimer.h"
#include "Common/Math/BezierSpline.h"

namespace FB
{
	class HandSlam;

	class HandSlam_Detect : public BaseFinalBossState
	{
	public:
		HandSlam_Detect(HandSlam& aParent);

		void Update() override;

		bool IsFinished() override;

		void OnEnter() override;
		void OnExit() override;

	private:
		HandSlam& myHandSlam;
		bool myIsFinished = false;
	};
}