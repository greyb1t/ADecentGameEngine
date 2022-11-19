#pragma once

#include "Scene.h"

namespace Engine
{
	class SpriteComponent;

	class LogoScene : public Scene
	{
		enum class eFadeState
		{
			FadeIn,
			Stay,
			FadeOut
		};

	public:
		bool Init(std::function<void()> aFinished);

		void Update(TimeStamp ts) override;

	private:
		Owned<ResourceReferences> myResourceRefences;

		std::function<void()> myFinished = nullptr;

		GameObject* myBackground = nullptr;
		GameObject* myTGALogo = nullptr;
		GameObject* myTeamLogo = nullptr;
		GameObject* myFmodLogo = nullptr;

		SpriteComponent* myTGAImage = nullptr;
		SpriteComponent* myTeamImage = nullptr;
		SpriteComponent* myFmodImage = nullptr;

		std::vector<SpriteComponent*> myImages;
		int myCurrentImageIndex = 0;
		SpriteComponent* myCurrentFadeImage = nullptr;

		eFadeState myFadeState = eFadeState::FadeIn;

		float myFadeDuration = 1.5f;
		float myFadeProgress = 0.0f;

		float myStayDuration = 3.0f;
		float myStayProgress = 0.0f;
	};
}