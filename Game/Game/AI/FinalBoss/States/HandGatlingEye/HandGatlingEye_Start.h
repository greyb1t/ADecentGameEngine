#pragma once

#include "AI/FinalBoss/States/FinalBossState.h"
#include "Utils/TickTimer.h"

namespace FB
{
	class HandGattlingEye;

	class HandGatlingEye_Start : public BaseFinalBossState
	{
	public:
		HandGatlingEye_Start(HandGattlingEye& aParent);

		void Update() override;

		bool IsFinished() override;

		void OnEnter() override;
		void OnExit() override;

	private:
		void GatlingLaserStartSound(const bool aEnable) const;

	private:
		HandGattlingEye& myEye;

		bool myIsFinished = false;
	};
}