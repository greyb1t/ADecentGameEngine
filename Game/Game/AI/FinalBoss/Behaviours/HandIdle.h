#pragma once

#include "Behaviour.h"
#include "AI/FinalBoss/States/FinalBossState.h"

namespace Engine
{
	class AnimatorComponent;
}

namespace FB
{
	class HandIdle : public Behaviour
	{
	public:
		HandIdle();

		void Update() override;

		bool IsFinished() const override;

		void OnEnter() override;
		void OnExit() override;

		void Reflect(Engine::Reflector& aReflector) override;

		bool IsAllowedToInterrupt() const override;

	private:
	};
}