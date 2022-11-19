#include "pch.h"
#include "LogoScene.h"
#include "Engine/GameObject\Components\SpriteComponent.h"
#include "Engine/GameObject\GameObject.h"
#include "Engine/ResourceManagement\Resources\TextureResource.h"
#include "Engine/Engine.h"
#include "Engine/ResourceReferences.h"

bool Engine::LogoScene::Init(std::function<void()> aFinished)
{
	if (!Scene::Init())
	{
		return false;
	}

	myResourceRefences = MakeOwned<ResourceReferences>();

	myResourceRefences->AddTexture("Assets\\Engine\\white.dds");
	myResourceRefences->AddTexture("Assets\\Engine\\TGA-Logo_1024x1024.dds");
	myResourceRefences->AddTexture("Assets\\Engine\\grouplogo_1024x1024.dds");

	myFinished = aFinished;

	myBackground = AddGameObject<GameObject>();
	myBackground->GetTransform().SetPosition(Vec3f(0.5f, 0.5f, 0.f));
	myBackground->GetTransform().SetScale(100.f);
	auto image = myBackground->AddComponent<SpriteComponent>("Assets\\Engine\\white.dds", 0);
	image->SetColor({ 0.0f, 0.0f, 0.0f, 1.0f });

	myTGALogo = AddGameObject<GameObject>();
	myTGALogo->GetTransform().SetPosition(Vec3f(0.5f, 0.5f, 0.f));
	myTGALogo->GetTransform().SetScale(0.7f);
	myTGAImage = myTGALogo->AddComponent<SpriteComponent>("Assets\\Engine\\TGA-Logo_1024x1024.dds", 0);
	myTGAImage->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
	myImages.push_back(myTGAImage);

	myTeamLogo = AddGameObject<GameObject>();
	myTeamLogo->GetTransform().SetPosition(Vec3f(0.5f, 0.5f, 0.f));
	myTeamImage = myTeamLogo->AddComponent<SpriteComponent>("Assets\\Engine\\grouplogo_1024x1024.dds", 0);
	myTeamImage->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
	myImages.push_back(myTeamImage);

	myFmodLogo = AddGameObject<GameObject>();
	myFmodLogo->GetTransform().SetPosition(Vec3f(0.5f, 0.5f, 0.f));
	myFmodImage = myFmodLogo->AddComponent<SpriteComponent>("Assets\\Engine\\fmod_logo.dds", 0);
	myFmodImage->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
	myImages.push_back(myFmodImage);

	myCurrentImageIndex = 0;
	myCurrentFadeImage = myImages[myCurrentImageIndex];

	myFadeProgress = myStayProgress = 0.0f;
	myFadeState = eFadeState::FadeIn;

	return true;
}

void Engine::LogoScene::Update(TimeStamp ts)
{
	Scene::Update(ts);

	const float deltaTime = ts;

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
					myFinished();
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
