#pragma once

#include "Component.h"
#include "Engine\Renderer\Material\MaterialInstance.h"

class GameObject;

namespace Engine
{
	class SpriteComponent;

	class LogoFader : public Component
	{
		enum class eFadeState
		{
			FadeIn,
			Stay,
			FadeOut
		};

	public:
		COMPONENT(LogoFader, "Logo Fader");

		LogoFader() = default;
		LogoFader(GameObject* aGameObject);

		void Start() override;

		void Execute(eEngineOrder aOrder) override;

		void Render() override;

		void Reflect(Reflector& aReflector) override;

	private:
		bool myHasStartedChangingScene = false;

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
		//float myFadeDuration = 0.5f;
		float myFadeProgress = 0.0f;

		float myStayDuration = 3.0f;
		//float myStayDuration = 0.3f;
		float myStayProgress = 0.0f;

		std::string myNextScenePath;
		std::string myNextSceneName;
	};
}