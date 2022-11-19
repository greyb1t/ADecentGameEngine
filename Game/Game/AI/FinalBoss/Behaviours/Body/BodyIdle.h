#pragma once

#include "Game/AI/FinalBoss/Behaviours/Behaviour.h"
#include "AI/FinalBoss/States/FinalBossState.h"

namespace Engine
{
	class AnimatorComponent;
}

namespace FB
{
	class FinalBossBody;

	class BodyIdle : public Behaviour
	{
	public:
		BodyIdle(FinalBossBody& aBody);

		void Update() override;

		bool IsFinished() const override;

		void OnEnter() override;
		void OnExit() override;

		void Reflect(Engine::Reflector& aReflector) override;

		bool IsAllowedToInterrupt() const override;

	private:
		FinalBossBody& myBody;
	};
}