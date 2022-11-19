#pragma once

#include "Game/AI/FinalBoss/Behaviours/Behaviour.h"
#include "AI/FinalBoss/States/FinalBossState.h"
#include "Utils/TickTimer.h"

namespace Engine
{
	class AnimatorComponent;
	class SpriteComponent;
	class VideoComponent;
}

namespace FB
{
	class FinalBossBody;

	class BodyDeath : public Behaviour
	{
	public:
		BodyDeath(FinalBossBody& aBody);

		void Update() override;

		bool IsFinished() const override;

		void OnEnter() override;
		void OnExit() override;

		void Reflect(Engine::Reflector& aReflector) override;

	private:
		void OnFinished();
		void PlayCreditsVideo();
		void MakePlayerInvincible();
		void KillSurroundingEnemies();

	private:
		FinalBossBody& myBody;
		bool myStartFading = false;
		GameObject* myWhiteSpriteGameObject = nullptr;
		Engine::SpriteComponent* myWhiteSprite = nullptr;
		float mySpriteAlpha = 0.f;
		bool myHasStartedChangingToMainMenu = false;

		Engine::VideoComponent* myEndCredits = nullptr;

		TickTimer myStartFadingTimer;
	};
}