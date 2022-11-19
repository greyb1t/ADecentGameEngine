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

	class HandDeath : public Behaviour
	{
	public:
		HandDeath(FinalBossHand& aHand, Engine::AnimatorComponent& aAnimator);

		void Update() override;

		bool IsFinished() const override;

		void OnEnter() override;
		void OnExit() override;

		void Reflect(Engine::Reflector& aReflector) override;

	private:
		void SpawnVFX();
		void DestroyHand();

	private:
		FinalBossHand& myHand;
		Engine::AnimatorComponent& myAnimator;
	};
}