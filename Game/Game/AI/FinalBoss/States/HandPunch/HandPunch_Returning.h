#pragma once

#include "AI/FinalBoss/States/FinalBossState.h"
#include "Common/Math/BezierSpline.h"
#include "Utils/TickTimer.h"

namespace FB
{
	class HandPunch;

	class HandPunch_Returning : public BaseFinalBossState
	{
	public:
		HandPunch_Returning(HandPunch& aParent);

		void Update() override;

		bool IsFinished() override;

		void OnEnter() override;
		void OnExit() override;

	private:
		HandPunch& myHandPunch;
		// TickTimer myTimer;
		BezierSpline myBezier;

		bool myIsFinished = false;
	};
}