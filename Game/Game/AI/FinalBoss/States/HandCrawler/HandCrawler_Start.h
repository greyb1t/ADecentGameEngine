#pragma once

#include "AI/FinalBoss/States/FinalBossState.h"
#include "Utils/TickTimer.h"

namespace Engine
{
	class Curve;
}

namespace FB
{
	class HandCrawler;

	class HandCrawler_Start : public BaseFinalBossState
	{
	public:
		HandCrawler_Start(HandCrawler& aParent);

		void Update() override;

		bool IsFinished() override;

		void OnEnter() override;
		void OnExit() override;

	private:
		HandCrawler& myCrawler;
		// TickTimer myTimer;
		float myTimeElapsed = 0.f;
		BezierSpline mySpline;
		Engine::Curve* myDecentSpeedCurve = nullptr;

		bool myIsFinished = false;
		bool myHasTriggeredAnimation = false;
	};
}