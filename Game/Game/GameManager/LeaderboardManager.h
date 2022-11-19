#pragma once

#define LEADERBOARD_COUNT 10

struct LeaderboardEntry
{
	float myTotalTime = FLT_MAX - 1.0f;

	float myMinutes = FLT_MAX - 1.0f;
	float mySeconds = FLT_MAX - 1.0f;
	float myMilliSeconds = FLT_MAX - 1.0f;
};

struct LeaderboardManager
{
	const std::string myLeaderboardPath = "Assets/Leaderboard.json";

	std::array<LeaderboardEntry, LEADERBOARD_COUNT> myEducatorsMode;
	std::array<LeaderboardEntry, LEADERBOARD_COUNT> myBeginner;
	std::array<LeaderboardEntry, LEADERBOARD_COUNT> myNormal;
	std::array<LeaderboardEntry, LEADERBOARD_COUNT> myHard;
	std::array<LeaderboardEntry, LEADERBOARD_COUNT> myNightmare;
	std::array<LeaderboardEntry, LEADERBOARD_COUNT> myImpossible;
	std::array<LeaderboardEntry, LEADERBOARD_COUNT> myStarMode;


	void AddScore(const float aTime, const float aDifficultyMultiplier);

	void Save();
	void Load();
	void Apply();

private:
	void SaveFrom(
		std::array<LeaderboardEntry, LEADERBOARD_COUNT>& anEntry,
		nlohmann::json& aJson,
		const std::string& aModeString
	);

	void LoadFrom(
		std::array<LeaderboardEntry, LEADERBOARD_COUNT>& anEntry,
		const nlohmann::json& aJson, 
		const std::string& aModeString);

	void ApplyFrom(
		std::array<LeaderboardEntry, LEADERBOARD_COUNT>& anEntry
	);

	void ConvertTotalTime(LeaderboardEntry& anEntry);

	void AddScoreInternal(
		std::array<LeaderboardEntry, LEADERBOARD_COUNT>& anEntry, 
		const float aTime
	);
};