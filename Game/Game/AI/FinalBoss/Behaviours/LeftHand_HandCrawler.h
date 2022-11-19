#pragma once

#include "Behaviour.h"
#include "AI/FinalBoss/StateSequencer.h"
#include "AI/FinalBoss/States/FinalBossState.h"
#include "HandCrawler.h"

namespace Engine
{
	class AnimatorComponent;
}

namespace FB
{
	class LeftPunchHand;

	class LeftHand_HandCrawler : public HandCrawler
	{
	public:
		LeftHand_HandCrawler(LeftPunchHand& aLeftHand);
		virtual ~LeftHand_HandCrawler() = default;

		LeftPunchHand& GetLeftHand();

	private:
		LeftPunchHand& myLeftHand;
	};
}