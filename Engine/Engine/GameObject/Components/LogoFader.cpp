#include "pch.h"
#include "LogoFader.h"
#include "Engine\Reflection\Reflector.h"
#include "Engine\GameObject\GameObject.h"
#include "SpriteComponent.h"
#include "Engine\Engine.h"
#include "Engine\Scene\SceneManager.h"

Engine::LogoFader::LogoFader(GameObject* aGameObject)
	: Component(aGameObject)
{
}

void Engine::LogoFader::Start()
{
	auto tgaLogo = myGameObject->GetScene()->FindGameObject("TGA Logo");
	myImages.push_back(tgaLogo->GetComponent<SpriteComponent>());

	myTeamLogo = myGameObject->GetScene()->FindGameObject("Group Logo");
	myImages.push_back(myTeamLogo->GetComponent<SpriteComponent>());

	myFmodLogo = myGameObject->GetScene()->FindGameObject("FMOD Logo");
	myImages.push_back(myFmodLogo->GetComponent<SpriteComponent>());

	myCurrentImageIndex = 0;
	myCurrentFadeImage = myImages[myCurrentImageIndex];

	myFadeProgress = myStayProgress = 0.0f;
	myFadeState = eFadeState::FadeIn;
}

void Engine::LogoFader::Execute(eEngineOrder aOrder)
{
	if (myHasStartedChangingScene)
	{
		return;
	}

	const float deltaTime = Time::DeltaTime;

	switch (myFadeState)
	{
	case eFadeState::FadeIn:
	{
		myFadeProgress += deltaTime;
		const float ratio = myFadeProgress / myFadeDuration;
		myCurrentFadeImage->SetAlpha(ratio);

		if (ratio > 1.0f)
		{
			myFadeProgress = myStayProgress = 0.0f;
			myCurrentFadeImage->SetAlpha(1.0f);
			myFadeState = eFadeState::Stay;
		}
	} break;

	case eFadeState::Stay:
	{
		myStayProgress += deltaTime;
		const float ratio = myStayProgress / myStayDuration;

		if (ratio > 1.0f)
		{
			myFadeProgress = myStayProgress = 0.0f;
			myFadeState = eFadeState::FadeOut;
		}
	} break;

	case eFadeState::FadeOut:
	{
		myFadeProgress += deltaTime;
		const float ratio = myFadeProgress / myFadeDuration;
		myCurrentFadeImage->SetAlpha(1.0f - ratio);

		if (ratio > 1.0f)
		{
			myFadeProgress = myStayProgress = 0.0f;
			myFadeState = eFadeState::FadeIn;

			myCurrentImageIndex++;
			if (myCurrentImageIndex >= myImages.size())
			{
				if (!myHasStartedChangingScene)
				{
					myHasStartedChangingScene = true;

					SceneHandle handle = GetEngine().GetSceneManager().LoadSceneAsync(
						myNextScenePath);

					GetEngine().GetSceneManager().SetNextScene(handle);
				}
			}
			else
			{
				myCurrentFadeImage = myImages[myCurrentImageIndex];
				myCurrentFadeImage->SetAlpha(0.0f);
			}
		}
	} break;

	default:
		break;
	}
}

void Engine::LogoFader::Render()
{
}

void Engine::LogoFader::Reflect(Reflector& aReflector)
{
	aReflector.Reflect(myNextScenePath, "Next Scene Path");
	aReflector.Reflect(myNextSceneName, "Next Scene Name");
}
