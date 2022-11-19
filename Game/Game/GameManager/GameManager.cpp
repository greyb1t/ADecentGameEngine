#include "pch.h"
#include "GameManager.h"

#include "Engine/GameObject/GameObject.h"
#include "Engine/Scene/SceneManager.h"

#include "Engine/Renderer/GraphicsEngine.h"
#include "Engine/Renderer/WindowHandler.h"

#include "Common/FileIO.h"
#include "Common/StringUtilities.h"
#include "Engine/GameObject/Components/AudioComponent.h"
#include "Engine/AudioManager.h"

static GameManager* myInstance = nullptr;

static const std::vector<std::string> globalLevels{
	"Assets/Scenes/UEDPIE_0_LVL01_GrassyPlains",
	"Assets/Scenes/UEDPIE_0_LVL02_GreenHill",
	"Assets/Scenes/UEDPIE_0_LVL03_DesertDunes",
	"Assets/Scenes/UEDPIE_0_LVL04_DesertHills"
};

bool GameManager::Start()
{
	if (!myInstance)
	{
		myInstance = DBG_NEW GameManager();
		return myInstance->InternalStart();
	}
	return false;
}

GameManager* GameManager::GetInstance()
{
	return myInstance;
}

void GameManager::Init()
{
	myDifficultyTimer = 0.f;

	InitLevels();
	InitGameSettings();
}

void GameManager::Reset()
{
	myDifficultyTimer = 0.f;
	myLevelChangeCount = 0;
	myTotalGameTime = 0.f;
	myMoney = 0;
	myEndlessCounter = 0;
	SetGameDifficulty(GameDifficulty::Beginner);
	InitLevels();
}

const GameDifficulty& GameManager::GetGameDifficulty() const
{
	return myGameDifficulty;
}

void GameManager::SetGameDifficulty(GameDifficulty aGameDifficulty)
{
	myGameDifficulty = aGameDifficulty;
}

void GameManager::IncreaseDifficulty()
{
	switch(GetGameDifficulty())
	{
		case GameDifficulty::Beginner:
			SetGameDifficulty(GameDifficulty::Easy);
			break;
		case GameDifficulty::Easy:
			SetGameDifficulty(GameDifficulty::Medium);
			break;
		case GameDifficulty::Medium:
			SetGameDifficulty(GameDifficulty::Hard);
			break;
		case GameDifficulty::Hard:
			SetGameDifficulty(GameDifficulty::Extreme);
			break;
		case GameDifficulty::Extreme:
			SetGameDifficulty(GameDifficulty::Haha);
			break;
		case GameDifficulty::Haha:
			SetGameDifficulty(GameDifficulty::Endless);
			break;
		case GameDifficulty::Endless:
			return;

		default: return;
	}

	myDifficultyTimer = 0.f;
}

void GameManager::ChangeLevel()
{
	if(myLevels.empty())
	{
		InitLevels();
	}

	const std::string& nextLevel = myLevels.back();

	auto& s = GetEngine().GetSceneManager();
	const Engine::SceneHandle handle = s.LoadSceneAsync(nextLevel);
	s.SetNextScene(handle);
	myCurrentLevel = nextLevel;

	std::cout << "Changing to level: " << nextLevel << std::endl;

	myLevels.pop_back();

	std::cout << "Level queue size: " << myLevels.size() << std::endl;

	myLevelChangeCount++;
	myStandby = true;
}

void GameManager::ChangeLevel(const std::string& aLevelName)
{
	auto& s = GetEngine().GetSceneManager();
	const Engine::SceneHandle handle = s.LoadSceneAsync(aLevelName);
	AudioManager::GetInstance()->StopAll();
	s.SetNextScene(handle);

	myCurrentLevel = aLevelName;
	myLevelChangeCount++;
}

void GameManager::ChangeToMainMenu()
{
	Reset();
	auto& s = GetEngine().GetSceneManager();
	const Engine::SceneHandle handle = s.LoadSceneAsync("Assets/Scenes/MainMenu");
	AudioManager::GetInstance()->StopAll();
	s.SetNextScene(handle);
}

void GameManager::ChangeToWinScreen()
{
	auto& s = GetEngine().GetSceneManager();
	const Engine::SceneHandle handle = s.LoadSceneAsync("Assets/Scenes/WinScene");
	AudioManager::GetInstance()->StopAll();
	s.SetNextScene(handle);
}

GameSettings& GameManager::GetGameSettings()
{
	return mySettings;
}

LeaderboardManager& GameManager::GetLeaderboard()
{
	return myLeaderboardManager;
}

float GameManager::GetDifficultyNormalized() const
{
	float difficultyTime = GetGameDifficulty()._to_index() * ourDifficultyDurationsInSeconds;
	float normalized = (difficultyTime + myDifficultyTimer) / (+GameDifficulty::Endless * ourDifficultyDurationsInSeconds);
	return CU::Clamp<float>(0.f, 1.f, normalized);
}

uint32_t GameManager::GetLevelChangeCount() const
{
	return myLevelChangeCount;
}

float GameManager::GetTotalGameTime() const
{
	return myTotalGameTime;
}

int32_t GameManager::GetMoney() const
{
	return myMoney;
}

void GameManager::SetMoney(const int32_t aMoney)
{
	myMoney = aMoney;
}

const std::string& GameManager::GetLevelName() const
{
	return myCurrentLevel;
}

void GameManager::Resume()
{
	myStandby = false;
}

void GameManager::SetShouldUpdateGameTime(bool aShouldUpdate)
{
	myShouldUpdateGameTime = aShouldUpdate;
}

void GameManager::SetShouldUpdateDifficultyTime(bool aShouldUpdate)
{
	myShouldUpdateDifficultyTime = aShouldUpdate;
}

int GameManager::GetEndlessCounter() const
{
	return myEndlessCounter;
}

void GameManager::IncreaseEndlessCounter()
{
	myEndlessCounter++;
}

bool GameManager::InternalStart()
{
	return true;
}

void GameManager::Update()
{
	if(myStandby)
		return;

	if(myShouldUpdateDifficultyTime)
		myDifficultyTimer += Time::DeltaTime;

	if(myShouldUpdateGameTime)
		myTotalGameTime += Time::DeltaTime;

	if(GetGameDifficulty() == +GameDifficulty::Endless)
		return;

	if(myDifficultyTimer > ourDifficultyDurationsInSeconds)
	{
		IncreaseDifficulty();
		std::cout << "Difficulty increased to " << GetGameDifficulty()._to_string() << std::endl;
	}
}

void GameManager::InitLevels()
{
	myLevels = globalLevels;
	ShuffleLevels();
	myLevels.insert(myLevels.begin(), "Assets/Scenes/UEDPIE_0_LVL05_Boss");
}

void GameManager::InitGameSettings()
{
	mySettings.Load();
	mySettings.Apply();

	myLeaderboardManager.Load();
	myLeaderboardManager.Apply();
}

void GameManager::ShuffleLevels()
{
	for(size_t i = myLevels.size() - 1; i >= 1; i--)
	{
		size_t r = Random::RandomUInt(0, i);
		CU::Swap(myLevels[r], myLevels[i]);
	}
}
