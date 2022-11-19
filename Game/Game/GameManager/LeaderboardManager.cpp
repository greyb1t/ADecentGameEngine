#include "pch.h"
#include "LeaderboardManager.h"

#include "Common/FileIO.h"

void LeaderboardManager::AddScore(const float aTime, const float aDifficultyMultiplier)
{
	// Sort
	Apply();

	if (aDifficultyMultiplier == 0.1f)
	{
		AddScoreInternal(myEducatorsMode, aTime);
	}
	else if (aDifficultyMultiplier == 0.75f)
	{
		AddScoreInternal(myBeginner, aTime);
	}
	else if (aDifficultyMultiplier == 1.0f)
	{
		AddScoreInternal(myNormal, aTime);
	}
	else if (aDifficultyMultiplier == 1.5f)
	{
		AddScoreInternal(myHard, aTime);
	}
	else if (aDifficultyMultiplier == 2.0f)
	{
		AddScoreInternal(myNightmare, aTime);
	}
	else if (aDifficultyMultiplier == 4.0f)
	{
		AddScoreInternal(myImpossible, aTime);
	}
	else if (aDifficultyMultiplier == 8.0f)
	{
		AddScoreInternal(myStarMode, aTime);
	}
	else
	{
		LOG_ERROR(LogType::Game) << "No difficulty multiplier matched that value";
		return;
	}

	// Sort again after inserting new value
	Apply();
}

void LeaderboardManager::Save()
{
	FileIO::RemoveReadOnly(myLeaderboardPath);
	std::ofstream file(myLeaderboardPath);

	nlohmann::json json;

	SaveFrom(myEducatorsMode, json, "EducatorsMode");
	SaveFrom(myBeginner, json, "Beginner");
	SaveFrom(myNormal, json, "Normal");
	SaveFrom(myHard, json, "Hard");
	SaveFrom(myNightmare, json, "Nightmare");
	SaveFrom(myImpossible, json, "Impossible");
	SaveFrom(myStarMode, json, "StarMode");

	if (file.is_open())
	{
		file << std::setw(4) << json;
		file.close();
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Failed to save game settings.";
	}
}

void LeaderboardManager::Load()
{
	if (!std::filesystem::exists(myLeaderboardPath))
	{
		Save();
	}

	std::ifstream file(myLeaderboardPath);

	if (!file.is_open())
	{
		LOG_ERROR(LogType::Game) << "Failed to load leaderboard from " << myLeaderboardPath;
		return;
	}

	nlohmann::json json = nlohmann::json::parse(file, nullptr, false, true);

	if (json.is_discarded())
	{
		LOG_ERROR(LogType::Game) << "Failed to parse leaderboard from " << myLeaderboardPath;
		return;
	}

	LoadFrom(myEducatorsMode, json, "EducatorsMode");
	LoadFrom(myBeginner, json, "Beginner");
	LoadFrom(myNormal, json, "Normal");
	LoadFrom(myHard, json, "Hard");
	LoadFrom(myNightmare, json, "Nightmare");
	LoadFrom(myImpossible, json, "Impossible");
	LoadFrom(myStarMode, json, "StarMode");
}

void LeaderboardManager::Apply()
{
	ApplyFrom(myEducatorsMode);
	ApplyFrom(myBeginner);
	ApplyFrom(myNormal);
	ApplyFrom(myHard);
	ApplyFrom(myNightmare);
	ApplyFrom(myImpossible);
	ApplyFrom(myStarMode);
}

void LeaderboardManager::SaveFrom(std::array<LeaderboardEntry, 10>& anEntry, nlohmann::json& aJson, const std::string& aModeString)
{
	for (int i = 0; i < LEADERBOARD_COUNT; i++)
	{
		aJson[aModeString][i]["TotalTime"] = anEntry[i].myTotalTime;
		aJson[aModeString][i]["Minutes"] = anEntry[i].myMinutes;
		aJson[aModeString][i]["Seconds"] = anEntry[i].mySeconds;
		aJson[aModeString][i]["MilliSeconds"] = anEntry[i].myMilliSeconds;
	}
}

void LeaderboardManager::LoadFrom(std::array<LeaderboardEntry, 10>& anEntry, const nlohmann::json& aJson, const std::string& aModeString)
{
	for (int i = 0; i < LEADERBOARD_COUNT; i++)
	{
		if (aJson.contains(aModeString) && i < (int)aJson[aModeString].size())
		{
			if (aJson[aModeString][i].contains("TotalTime"))
				anEntry[i].myTotalTime = aJson[aModeString][i]["TotalTime"].get<float>();

			if (aJson[aModeString][i].contains("Minutes"))
				anEntry[i].myMinutes = aJson[aModeString][i]["Minutes"].get<float>();

			if (aJson[aModeString][i].contains("Seconds"))
				anEntry[i].mySeconds = aJson[aModeString][i]["Seconds"].get<float>();

			if (aJson[aModeString][i].contains("MilliSeconds"))
				anEntry[i].myMilliSeconds = aJson[aModeString][i]["MilliSeconds"].get<float>();
		}
	}
}

void LeaderboardManager::ApplyFrom(std::array<LeaderboardEntry, LEADERBOARD_COUNT>& anEntry)
{
	for (int i = 0; i < LEADERBOARD_COUNT; i++)
	{
		for (int j = 0; j < LEADERBOARD_COUNT; j++)
		{
			if (j == i)
				continue;

			if (anEntry[i].myTotalTime < anEntry[j].myTotalTime)
			{
				std::swap(anEntry[i], anEntry[j]);
			}
		}
	}

	for (int i = 0; i < LEADERBOARD_COUNT; i++)
	{
		ConvertTotalTime(anEntry[i]);
	}
}

void LeaderboardManager::ConvertTotalTime(LeaderboardEntry& anEntry)
{
	double dSeconds = 0.0f;
	double dMilsecs = std::modf(static_cast<double>(anEntry.myTotalTime), &dSeconds);

	int seconds = static_cast<int>(dSeconds);
	int milsecs = static_cast<int>(dMilsecs * 100.0);

	int minutes = 0;

	if (seconds > 60)
	{
		minutes = seconds / 60;
		seconds %= 60;
	}

	anEntry.myMinutes = minutes;
	anEntry.mySeconds = seconds;
	anEntry.myMilliSeconds = milsecs;
}

void LeaderboardManager::AddScoreInternal(std::array<LeaderboardEntry, LEADERBOARD_COUNT>& anEntry, const float aTime)
{
	for (int i = 0; i < anEntry.size(); i++)
	{
		if (anEntry[i].myTotalTime > FLT_MAX * .9f)
		{
			anEntry[i].myTotalTime = aTime;
			break;
		}

		if (aTime < anEntry[i].myTotalTime)
		{
			std::swap(anEntry.back(), anEntry[i]);
			anEntry[i].myTotalTime = aTime;
			break;
		}
	}
}
