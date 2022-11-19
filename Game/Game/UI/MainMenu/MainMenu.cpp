#include "pch.h"
#include "MainMenu.h"

void MainMenu::Start()
{
	MenuComponent::Start();

	UI::AddEvent("Play", [this](GameObject* aButtonObject) { Play(aButtonObject); });
	UI::AddEvent("LevelSelect", [this](GameObject* aButtonObject) { LevelSelect(aButtonObject); });
	UI::AddEvent("HowToPlay", [this](GameObject* aButtonObject) { HowToPlay(aButtonObject); });
	UI::AddEvent("Options", [this](GameObject* aButtonObject) { Options(aButtonObject); });
	UI::AddEvent("Credits", [this](GameObject* aButtonObject) { Credits(aButtonObject); });
	UI::AddEvent("ExitGame", [this](GameObject* aButtonObject) { ExitGame(aButtonObject); });

}

void MainMenu::Reflect(Engine::Reflector& aReflector)
{
	
}

void MainMenu::Play(GameObject* aButtonObject)
{
	myMenuHandler->EnablePlay();
	LOG_INFO(LogType::Game) << "Clicked Play";
}

void MainMenu::LevelSelect(GameObject* aButtonObject)
{
	myMenuHandler->EnableLevelSelect();
	LOG_INFO(LogType::Game) << "Clicked Level Select";
}

void MainMenu::HowToPlay(GameObject* aButtonObject)
{
	myMenuHandler->EnableHowToPlay();
	LOG_INFO(LogType::Game) << "Clicked How To Play";
}

void MainMenu::Options(GameObject* aButtonObject)
{
	myMenuHandler->EnableOptions();
	LOG_INFO(LogType::Game) << "Clicked Options";
}

void MainMenu::Credits(GameObject* aButtonObject)
{
	myMenuHandler->EnableCredits();
	LOG_INFO(LogType::Game) << "Clicked Credits";
}

void MainMenu::ExitGame(GameObject* aButtonObject)
{
#ifndef _RETAIL
	LOG_INFO(LogType::Game) << "Clicked Exit Game";
#else
	PostQuitMessage(0);
#endif
}
