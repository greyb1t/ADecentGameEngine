#include "pch.h"
#include "Play.h"

#include "Engine/GameObject/Components/TextComponent.h"
#include "Engine/GameObject/Components/SpriteComponent.h"
#include "Engine/GameObject/Components/AudioComponent.h"
#include "Engine/GameObject/Components/2DComponents/ButtonComponent.h"
#include "Engine/GameObject/Components/2DComponents/SliderComponent.h"

#include "Engine/ResourceManagement/Resources/TextureResource.h"

#include "Leaderboard.h"
#include "VideoHandler.h"

#include "Game/AI/LevelBoss/LevelBoss.h"

void Play::Start()
{
	InitButtonCallbacks();
	InitSliderStartValues();

	myAudioComponent = myGameObject->AddComponent<Engine::AudioComponent>();
	myAudioComponent->AddEvent("StartGame", "event:/CINE/Intro");
}

void Play::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myVideoHandlerRef, "Video Handler");

	aReflector.Reflect(myDifficultyRef, "Difficulty Slider");
	aReflector.Reflect(myDifficultyTextRef, "Difficulty Text");
	aReflector.Reflect(myDifficultySpriteRef, "Difficulty Sprite");
	aReflector.Reflect(myStarModeSpriteRef, "Star Mode Sprite");
	aReflector.Reflect(myLeaderboardRef, "Leaderboard Ref");

	aReflector.Header("Difficulty modifiers");
	aReflector.Reflect(myDifficulties, "Difficulties");
}

void Play::OnEnable()
{
	if (myLeaderboard)
	{
		auto gm = GameManager::GetInstance();
		auto& lb = gm->GetLeaderboard();

		lb.Load();

		myLeaderboard->ApplyFromLeaderboardManager();
	}
}

void Play::OnDisable()
{
	if (myLeaderboard)
	{
		auto gm = GameManager::GetInstance();
		auto& lb = gm->GetLeaderboard();

		lb.Save();
	}
}

void Play::InitButtonCallbacks()
{
	UI::AddEvent("Difficulty", [this](GameObject* aKnobObject, const float aSliderValue) { Difficulty(aSliderValue); });
	UI::AddEvent("SaveDifficulty", [this](GameObject* aKnobObject) { SaveDifficulty(); });
	UI::AddEvent("StartGame", [this](GameObject* aKnobObject) { StartGame(); });
	UI::AddEvent("ClearSelectedLevel", [this](GameObject* aKnobObject) { 
		GameManager::GetInstance()->myStartedOnBossLevel = false;
		myMenuHandler->ClearSelectedLevel(); 
		});
}

void Play::InitSliderStartValues()
{
	auto& settings = GameManager::GetInstance()->GetGameSettings();

	if (myDifficultyRef && myDifficultyRef.Get())
	{
		myDifficultySlider = myDifficultyRef.Get()->GetComponent<Engine::SliderComponent>();
		myDifficultySlider->SetValue(settings.myDifficultyMultiplier);
	}

	if (myDifficultyTextRef && myDifficultyTextRef.Get())
	{
		myDifficultyText = myDifficultyTextRef.Get()->GetComponent<Engine::TextComponent>();
	}

	if (myDifficultySpriteRef && myDifficultySpriteRef.Get())
	{
		myDifficultySprite = myDifficultySpriteRef.Get()->GetComponent<Engine::SpriteComponent>();
	}

	if (myLeaderboardRef && myLeaderboardRef.Get())
	{
		if (myLeaderboard = myLeaderboardRef.Get()->GetComponent<Leaderboard>())
		{
			myLeaderboard->ApplyFromLeaderboardManager();
		}
	}

	auto gm = GameManager().GetInstance();
	if (gm->GetGameSettings().myHasUnlockedStarMode)
	{
		if (myStarModeSpriteRef && myStarModeSpriteRef->IsValid())
		{
			myStarModeDifficulty.mySprite = myStarModeSpriteRef;
		}
		myDifficulties.push_back(myStarModeDifficulty);
	}

	SetDifficulty(settings.myDifficultyMultiplier);
}

void Play::Difficulty(const float aSliderValue)
{
	if (myDifficulties.size() < 2)
		return;

	float value = CU::Lerp(myDifficulties[0].myMultiplier, myDifficulties.back().myMultiplier, aSliderValue);

	SetDifficulty(value);

	if (myLeaderboard)
	{
		myLeaderboard->ApplyFromLeaderboardManager();
	}
}

void Play::SetDifficulty(const float aMultiplierValue)
{
	if (!myDifficultySlider || !myDifficultySprite || !myDifficultyText)
		return;

	if (myDifficulties.size() < 2)
		return;

	DifficultySettings difficulty = myDifficulties[0];
	float difference = std::abs(difficulty.myMultiplier - aMultiplierValue);
	for (int i = 1; i < myDifficulties.size(); i++)
	{
		float check = std::abs(myDifficulties[i].myMultiplier - aMultiplierValue);
		if (check < difference)
		{
			difficulty = myDifficulties[i];
			difference = check;
		}
	}

	myCurrentDifficulty = difficulty;

	float normalized = (myDifficulties.back().myMultiplier - myCurrentDifficulty.myMultiplier) / (myDifficulties.back().myMultiplier - myDifficulties[0].myMultiplier);
	normalized = 1.0f - normalized;
	//float normalized = static_cast<float>(index) / static_cast<float>(myDifficulties.size() - 1);

	auto& settings = GameManager::GetInstance()->GetGameSettings();
	settings.myDifficultyMultiplier = myCurrentDifficulty.myMultiplier;

	{
		std::stringstream ss;
		ss << difficulty.myLabel << " - " << std::fixed << std::setprecision(0) << (100.f * difficulty.myMultiplier) << "%";
		std::string s = ss.str();

		myDifficultyText->SetText(s);
	}

	{
		if (difficulty.mySprite && difficulty.mySprite->IsValid())
		{
			myDifficultySprite->SetSprite(difficulty.mySprite);
		}
		else
		{
			myDifficultySprite->ClearSprite();
		}
	}

	myDifficultySlider->SetValue(normalized);
}

void Play::SaveDifficulty()
{
	auto& settings = GameManager::GetInstance()->GetGameSettings();
	settings.Save();
}

void Play::StartGame()
{
	if (myAudioComponent)
	{
		myAudioComponent->StopEvent("Menu");
		myAudioComponent->PlayEvent("StartGame");
	}

	auto gm = GameManager::GetInstance();
	gm->Reset();

	Main::CueResetPlayer();
	LevelBoss::ResetBoss();

	if (myVideoHandlerRef && myVideoHandlerRef.Get())
	{
		if (auto handler = myVideoHandlerRef.Get()->GetComponent<VideoHandler>())
		{
			const std::string& path = myMenuHandler->GetSelectedLevel();
			handler->StartGame(path);
		}
	}
}

void DifficultySettings::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myLabel, "Label");
	aReflector.Reflect(myMultiplier, "Multiplier");
	aReflector.Reflect(mySprite, "Sprite");
}