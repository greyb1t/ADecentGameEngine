#pragma once

#include "AI/FinalBoss/States/FinalBossState.h"
#include "Utils/TickTimer.h"
#include "Common/Math/BezierSpline.h"

namespace FB
{
	class HandPunch;

	class HandPunch_Buildup : public BaseFinalBossState
	{
	public:
		HandPunch_Buildup(HandPunch& aParent);

		void Update() override;

		bool IsFinished() override;

		void OnEnter() override;
		void OnExit() override;

	private:
		HandPunch& myHandPunch;
		TickTimer myTimer;
		BezierSpline myBezier;
	};
}