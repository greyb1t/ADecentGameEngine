#pragma once

#include "Game/AI/FinalBoss/Behaviours/Behaviour.h"
#include "AI/FinalBoss/States/FinalBossState.h"
#include "Utils/TickTimer.h"

namespace Engine
{
	class AnimatorComponent;
}

namespace FB
{
	class FinalBossBody;

	class BodyIntro : public Behaviour
	{
	public:
		BodyIntro(FinalBossBody& aBody);

		void Update() override;

		bool IsFinished() const override;

		void OnEnter() override;
		void OnExit() override;

		void Reflect(Engine::Reflector& aReflector) override;

	private:
		FinalBossBody& myBody;
		bool myIsFinished = false;

		TickTimer myFailsafeTimer;
	};
}