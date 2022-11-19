#pragma once

#include "AI/LevelBoss/State/StateVars.h"
#include "GameSettings.h"
#include "LeaderboardManager.h"

BETTER_ENUM(GameDifficulty, int,
	Beginner,
	Easy,
	Medium,
	Hard,
	Extreme,
	Haha,
	Endless
);

class GameManagerUpdater;

class GameManager
{
public:
	static bool Start();

	static GameManager* GetInstance();

	void Init();
	void Reset();

	const GameDifficulty& GetGameDifficulty() const;
	void SetGameDifficulty(GameDifficulty aGameDifficulty);
	void IncreaseDifficulty();

	void ChangeLevel();
	void ChangeLevel(const std::string& aLevelName);
	void ChangeToMainMenu();
	void ChangeToWinScreen();

	GameSettings& GetGameSettings();
	LeaderboardManager& GetLeaderboard();

	float GetDifficultyNormalized() const;
	uint32_t GetLevelChangeCount() const;
	float GetTotalGameTime() const;
	
	int32_t GetMoney() const;
	void SetMoney(const int32_t aMoney);

	const std::string& GetLevelName() const;

	void Resume();

	void SetShouldUpdateGameTime(bool aShouldUpdate);
	void SetShouldUpdateDifficultyTime(bool aShouldUpdate);

	int GetEndlessCounter() const;
	void IncreaseEndlessCounter();

	bool myStartedOnBossLevel = false;

private:
	bool InternalStart();

	void Update();

	void InitLevels();
	void InitGameSettings();
	void ShuffleLevels();

	GameSettings mySettings;
	LeaderboardManager myLeaderboardManager;

	GameDifficulty myGameDifficulty = GameDifficulty::Beginner;
	std::vector<LevelBossState> myActiveBossStates;

	float myDifficultyTimer = 0.f;
	int myEndlessCounter = 0.f;
	std::vector<std::string> myLevels;
	std::string myCurrentLevel;
	uint32_t myLevelChangeCount = 0;
	float myTotalGameTime = 0.f;
	int32_t myMoney = 0;
	bool myStandby = false;

	static constexpr float ourDifficultyDurationsInSeconds = 3.f * 60.f;

	friend class GameManagerUpdater;

	bool myShouldUpdateGameTime = true;
	bool myShouldUpdateDifficultyTime = true;
};