#pragma once

#include "AI/FinalBoss/States/FinalBossState.h"
#include "Utils/TickTimer.h"

namespace FB
{
	class HandGattlingEye;

	class HandGatlingEye_End : public BaseFinalBossState
	{
	public:
		HandGatlingEye_End(HandGattlingEye& aParent);

		void Update() override;

		bool IsFinished() override;

		void OnEnter() override;
		void OnExit() override;

	private:
		HandGattlingEye& myEye;
		bool myIsFinished = false;
	};
}