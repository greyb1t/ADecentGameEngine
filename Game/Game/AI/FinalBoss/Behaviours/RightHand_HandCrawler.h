#pragma once

#include "Behaviour.h"
#include "AI/FinalBoss/StateSequencer.h"
#include "AI/FinalBoss/States/FinalBossState.h"
#include "HandCrawler.h"

namespace FB
{
	class RightProjectileHand;

	class RightHand_HandCrawler : public HandCrawler
	{
	public:
		RightHand_HandCrawler(RightProjectileHand& aLeftHand);
		virtual ~RightHand_HandCrawler() = default;

		RightProjectileHand& GetRightHand();

	private:
		RightProjectileHand& myRightHand;
	};
}