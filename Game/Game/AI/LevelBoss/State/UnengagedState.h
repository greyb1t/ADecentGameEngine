#pragma once

#include "BaseState.h"

namespace LevelBossStates
{
	class UnengagedState : public LBS::BaseState
	{
	public:
		UnengagedState() = delete;
		UnengagedState(LevelBoss& aBoss);

		void Update() override;

	private:

	};
}

