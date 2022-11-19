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

	BETTER_ENUM(FinalBossShockwaveState, int,
		Start,
		Loop,
		End);

	class BodyShockwaveScream : public Behaviour
	{
	public:
		BodyShockwaveScream(FinalBossBody& aBody);

		void Update() override;

		bool IsFinished() const override;

		void OnEnter() override;
		void OnExit() override;

		void Reflect(Engine::Reflector& aReflector) override;

	private:
		void EmitShockwave();
		void UpdateStart();
		void UpdateLoop();
		void UpdateEnd();
		void ShockwaveSound(const bool aEnable) const;

	private:
		FinalBossBody& myBody;
		FinalBossShockwaveState myState = FinalBossShockwaveState::Start;
		bool myIsFinished = false;
		TickTimer myLoopTimer;

		int myEmitCounter = 0;
	};
}