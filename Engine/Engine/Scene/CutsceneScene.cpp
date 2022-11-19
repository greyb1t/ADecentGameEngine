#include "pch.h"
#include "CutsceneScene.h"
#include "Engine/GameObject\GameObject.h"
#include "Engine/GameObject\Components\VideoComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameObject\Components\SpriteComponent.h"
#include "Engine/AnimationCurve\CurveManager.h"
#include "Engine/AnimationCurve\Curve.h"
#include "Engine/AudioManager.h"

bool Engine::CutsceneScene::Init(
	const Path& aCutscenePath,
	const std::string& aAudioEventPath,
	std::function<void()> aOnFinished)
{
	if (!Scene::Init())
	{
		return false;
	}

	AudioManager::GetInstance()->StopAll();

	//AudioManager::GetInstance()->PlayEvent2D(aAudioEventPath, "CutsceneAudio");

	myOnFinished = aOnFinished;

	myVideoGameObject = AddGameObject<GameObject>();
	myVideoGameObject->GetTransform().SetPosition({ 0.5f, 0.5f, 0.f });

	//myVideoComponent = myVideoGameObject->AddComponent<VideoComponent>(
	//	aCutscenePath.ToString(), 0);
	myVideoComponent->MoveToUIScene();
	myVideoComponent->Play(false);
	//AudioManager::GetInstance()->Stop("Foley Player (2D)");

	mySkipSpriteGameObject = AddGameObject<GameObject>();
	mySkipSpriteComponent = mySkipSpriteGameObject->AddComponent<SpriteComponent>(
		"Assets/Sprites/UI/main menu/UI_Skip_01_20G5M.dds", 1);

	myTransparentColor = C::Vector4f(1.f, 1.f, 1.f, 0.f);
	myWhiteColor = C::Vector4f(1.f, 1.f, 1.f, 1.f);

	mySkipSpriteComponent->SetColor(myTransparentColor);

	mySkipSpriteGameObject->GetTransform().SetPosition(Vec3f(0.9f, 0.9f, 0.f));

	assert(false && "get back the curves");
	// mySkipFadeInCurve = &GetEngine().GetCurveManager().GetCurve("CutsceneSkipSpriteFadeIn");
	// mySkipFadeOutCurve = &GetEngine().GetCurveManager().GetCurve("CutsceneSkipSpriteFadeOut");

	return true;
}

void Engine::CutsceneScene::Update(TimeStamp ts)
{
	Scene::Update(ts);

	UpdateSkipSpriteFade(ts);

	auto& input = GetEngine().GetInputManager();

	if (mySkipFadeDownTimer.IsStarted())
	{
		if (mySkipFadeDownTimer.IsFinished())
		{
			// mySkipSpriteComponent->SetColor(myTransparentColor);

			if (mySkipSpriteFadeStatus != FadeStatus::FadeOut)
			{
				// Fade our skip sprite
				mySkipFadeDownTimer.Init(2.f);
				mySkipFadeDownTimer.Start();
				mySkipSpriteFadeStatus = FadeStatus::FadeOut;
			}
		}
		else
		{
			// mySkipSpriteComponent->SetColor(myWhiteColor);

			// If we clicked Enter while the skip timer was active, do the skip
			if (input.IsKeyDown(C::KeyCode::Enter))
			{
				myVideoComponent->Stop();

				// stop the cutscene sounds
				AudioManager::GetInstance()->StopAll();

				myOnFinished();
			}
		}
	}

	if (input.IsAnyKeyDown())
	{
		if (mySkipSpriteFadeStatus != FadeStatus::FadeIn)
		{
			// Fade in skip sprite
			mySkipFadeDownTimer.Init(2.f);
			mySkipFadeDownTimer.Start();
			mySkipSpriteFadeStatus = FadeStatus::FadeIn;
		}
	}

	if (myVideoComponent->IsFinished())
	{
		if (myOnFinished)
		{
			myOnFinished();

			// to avoid calling it again
			myOnFinished = nullptr;
		}
	}
}

void Engine::CutsceneScene::UpdateSkipSpriteFade(TimeStamp ts)
{
	mySkipFadeDownTimer.Update(ts);

	const float t = mySkipFadeDownTimer.GetPercentage();

	switch (mySkipSpriteFadeStatus)
	{
		case CutsceneScene::FadeStatus::FadeIn:
			mySkipSpriteComponent->SetAlpha(mySkipFadeInCurve->Evaluate(t));
			break;
		case CutsceneScene::FadeStatus::FadeOut:
			mySkipSpriteComponent->SetAlpha(mySkipFadeOutCurve->Evaluate(t));
			break;
		case CutsceneScene::FadeStatus::Nothing:
			break;
		default:
			assert(false);
			break;
	}
}
