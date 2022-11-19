#include "pch.h"
#include "LevelSelect.h"

#include "ImageReferencer.h"

#include "VideoHandler.h"
#include "Game/AI/LevelBoss/LevelBoss.h"

void LevelSelect::Start()
{
	MenuComponent::Start();

	UI::AddEvent("GrassyPlains", [this](GameObject* aButtonObject) { PlayLevel("Assets/Scenes/UEDPIE_0_LVL01_GrassyPlains"); });
	UI::AddEvent("GreenHill", [this](GameObject* aButtonObject) { PlayLevel("Assets/Scenes/UEDPIE_0_LVL02_GreenHill"); });
	UI::AddEvent("DesertDunes", [this](GameObject* aButtonObject) { PlayLevel("Assets/Scenes/UEDPIE_0_LVL03_DesertDunes"); });
	UI::AddEvent("DesertHills", [this](GameObject* aButtonObject) { PlayLevel("Assets/Scenes/UEDPIE_0_LVL04_DesertHills"); });
	UI::AddEvent("Boss", [this](GameObject* aButtonObject) 
		{ 
			GameManager::GetInstance()->myStartedOnBossLevel = true;
			PlayLevel("Assets/Scenes/UEDPIE_0_LVL05_Boss");
		});

	UI::AddEvent("EnableImagePreview", [this](GameObject* aButtonObject) { EnableImagePreview(aButtonObject); });
	UI::AddEvent("DisableImagePreview", [this](GameObject* aButtonObject) { DisableImagePreview(aButtonObject); });
}

void LevelSelect::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myVideoHandler, "Video Handler");
}

void LevelSelect::PlayLevel(const std::string aPath)
{
	LOG_INFO(LogType::Game) << "Started level " << aPath;
	
	myMenuHandler->SetSelectedLevel(aPath);
	myMenuHandler->EnablePlay();
}

void LevelSelect::EnableImagePreview(GameObject* aButtonObject)
{
	if (!aButtonObject)
		return;

	if (auto ref = aButtonObject->GetComponent<ImageReferencer>())
	{
		ref->EnableImage();
	}
}

void LevelSelect::DisableImagePreview(GameObject* aButtonObject)
{
	if (!aButtonObject)
		return;

	if (auto ref = aButtonObject->GetComponent<ImageReferencer>())
	{
		ref->DisableImage();
	}
}
