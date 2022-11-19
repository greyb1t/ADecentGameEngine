#pragma once

#include "Behaviour.h"
#include "AI/FinalBoss/States/FinalBossState.h"

namespace Engine
{
	class AnimatorComponent;
}

namespace FB
{
	class FinalBossHand;

	class HandIntro : public Behaviour
	{
	public:
		HandIntro(FinalBossHand& aHand);

		void Update() override;

		bool IsFinished() const override;

		void OnEnter() override;
		void OnExit() override;

		void Reflect(Engine::Reflector& aReflector) override;

		bool IsAllowedToMoveExternally() const override;

	private:

	private:
		FinalBossHand& myHand;
		bool myIsFinished = false;
	};
}