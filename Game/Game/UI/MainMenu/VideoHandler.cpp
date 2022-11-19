#include "pch.h"
#include "VideoHandler.h"

#include "MenuHandler.h"

#include "Engine/GameObject/Components/VideoComponent.h"
#include "Engine/GameObject/Components/AudioComponent.h"
#include "Engine/GameObject/Components/MusicManager.h"
#include "Engine/AudioManager.h"

void VideoHandler::Awake()
{
	if (auto ac = myGameObject->GetComponent<Engine::AudioComponent>())
	{
		ac->AddEvent("SplashScreen", "event:/UI/CogDogSplash");

		ac->PlayEvent("SplashScreen");
		ac->StopEvent("SplashScreen");
	}
}
void VideoHandler::Start()
{
	const bool gameStarted = GameManager::GetInstance()->GetGameSettings().myGameHasStartedFlag;

	if (auto ac = myGameObject->GetComponent<Engine::AudioComponent>())
	{
		if (!gameStarted)
		{
			ac->PlayEvent("SplashScreen");
		}
	}

	if (mySplashVideo && mySplashVideo.Get())
	{

		mySplashVideo.Get()->SetActive(!gameStarted);

		if (auto video = mySplashVideo.Get()->GetComponent<Engine::VideoComponent>())
		{
			video->SetCallback([this]()
				{
					EnableMainMenu();
					mySplashSkipped = true;
				});
		}
	}

	if (myBackgroundVideo && myBackgroundVideo.Get())
	{
		myBackgroundVideo.Get()->SetActive(gameStarted);

		if (gameStarted)
		{
			if (auto video = myBackgroundVideo.Get()->GetComponent<Engine::VideoComponent>())
			{
				myOldFuck = video->myLoopStartTime;
				video->myLoopStartTime = 0.0f;
				video->SeekToSeconds(video->myLoopStartTime);
				video->Play(true);
				myShitFuck = true;
			}
		}
	}

	myLevelIsLoading = false;
	myStartGameVideoPlaying = false;
	if (myStartGameVideo && myStartGameVideo.Get())
	{
		myStartGameVideo.Get()->SetActive(false);

		if (auto video = myStartGameVideo.Get()->GetComponent<Engine::VideoComponent>())
		{
			video->SetCallback([this]() { LoadLevel(""); });
		}
	}

	if (gameStarted)
	{
		auto mm = myGameObject->GetSingletonComponent<Engine::MusicManager>();
		if (mm)
		{
			mm->Play();
		}
	}
}

void VideoHandler::Execute(Engine::eEngineOrder aOrder)
{
	auto& input = GetEngine().GetInputManager();
	if (input.IsKeyDown(Common::KeyCode::Escape))
	{
		if (!myLevelIsLoading)
		{
			if (myStartGameVideoPlaying)
			{
				if (auto ac = myGameObject->GetComponent<Engine::AudioComponent>())
				{
				}
				if (myMenuHandler && myMenuHandler.Get())
				{
					auto handler = myMenuHandler.Get()->GetComponent<MenuHandler>();
					LoadLevel(handler->GetSelectedLevel());
				}
			}
			else
			{
				EnableMainMenu();
			}
		}
	}

	if (myShitFuck)
	{
		if (auto video = myBackgroundVideo.Get()->GetComponent<Engine::VideoComponent>())
		{
			video->myLoopStartTime = 5.0f;
			video->Play(true);
		}
		myShitFuck = false;
	}
}

void VideoHandler::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(mySplashVideo, "Splash Video");
	aReflector.Reflect(myBackgroundVideo, "Background Video");
	aReflector.Reflect(myStartGameVideo, "Start Game Video");
	aReflector.Reflect(myMenuHandler, "Menu Handler");
}

void VideoHandler::EnableMainMenu()
{
	if (!mySplashSkipped)
	{
		auto am = AudioManager::GetInstance();
		if (am)
		{
			am->StopAll();
		}

		auto mm = myGameObject->GetSingletonComponent<Engine::MusicManager>();
		if (mm)
		{
			mm->Play();
		}

		mySplashSkipped = true;
	}

	if (mySplashVideo && mySplashVideo.Get())
	{
		mySplashVideo.Get()->SetActive(false);
	}

	if (myBackgroundVideo && myBackgroundVideo.Get())
	{
		myBackgroundVideo.Get()->SetActive(true);
	}

	if (myMenuHandler && myMenuHandler.Get())
	{
		if (auto menu = myMenuHandler.Get()->GetComponent<MenuHandler>())
		{
			menu->AnimateIntro();
		}
	}

	mySplashSkipped = true;
}

void VideoHandler::StartGame(const std::string& aPath)
{
	myStartGameVideoPlaying = true;

	if (mySplashVideo && mySplashVideo.Get())
	{
		mySplashVideo.Get()->SetActive(false);
	}

	if (myBackgroundVideo && myBackgroundVideo.Get())
	{
		myBackgroundVideo.Get()->SetActive(false);
	}

	if (myStartGameVideo && myStartGameVideo.Get())
	{
		myStartGameVideo.Get()->SetActive(true);

		auto mm = myGameObject->GetSingletonComponent<Engine::MusicManager>();
		if (mm)
		{
			mm->Stop();
		}

		auto video = myStartGameVideo.Get()->GetComponent<Engine::VideoComponent>();
		if (video)
		{
			video->SetCallback([&]() { LoadLevel(aPath); });
		}
	}

	if (auto handler = myMenuHandler.Get()->GetComponent<MenuHandler>())
	{
		handler->DisableAll();
	}
}

void VideoHandler::DisableAll()
{
	if (mySplashVideo && mySplashVideo.Get())
	{
		mySplashVideo.Get()->SetActive(false);
	}

	if (myBackgroundVideo && myBackgroundVideo.Get())
	{
		myBackgroundVideo.Get()->SetActive(false);
	}

	if (myStartGameVideo && myStartGameVideo.Get())
	{
		myStartGameVideo.Get()->SetActive(false);
	}
}

void VideoHandler::SetLevelIsLoading()
{
	myLevelIsLoading = true;
}

void VideoHandler::LoadLevel(const std::string& aPath)
{
	if (auto handler = myMenuHandler.Get()->GetComponent<MenuHandler>())
	{
		if (myStartGameVideo && myStartGameVideo.Get())
		{
			myStartGameVideo.Get()->SetActive(false);
		}

		handler->EnableLoadingScreen();

		if (aPath != "")
		{
			GameManager::GetInstance()->ChangeLevel(aPath);
		}
		else 
		{
			GameManager::GetInstance()->ChangeLevel();
		}

		myLevelIsLoading = true;
	}
}
