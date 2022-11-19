#pragma once

#include "AI/FinalBoss/States/FinalBossState.h"
#include "Utils/TickTimer.h"

namespace FB
{
	class HandCrawler;

	class HandCrawler_End : public BaseFinalBossState
	{
	public:
		HandCrawler_End(HandCrawler& aParent);

		void Update() override;

		bool IsFinished() override;

		void OnEnter() override;
		void OnExit() override;

	private:
		HandCrawler& myParent;

		bool myIsFinished = false;
	};
}