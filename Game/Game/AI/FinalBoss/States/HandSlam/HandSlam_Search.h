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

	class HandSlam_Search : public BaseFinalBossState
	{
	public:
		HandSlam_Search(HandSlam& aParent);

		void Update() override;

		bool IsFinished() override;

		void OnEnter() override;
		void OnExit() override;

	private:
		void PlaySound();

	private:
		HandSlam& myHandSlam;
		TickTimer myTimer;
		BezierSpline myBezier;
		Vec3f myStartPos;
		Engine::Curve* mySpeedCurve = nullptr;

		bool myPlayedSearchSound = false;
	};
}