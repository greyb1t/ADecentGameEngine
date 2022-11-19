#include "pch.h"
#include "Leaderboard.h"

#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/TextComponent.h"

void Leaderboard::Awake()
{
	int childCount = GetTransform().GetChildren().size();
	for (int i = 0; i < childCount; i++)
	{
		auto childTransform = GetTransform().GetChildByIndex(i);
		if (!childTransform->GetChildren().empty())
		{
			if (auto superChildObject = childTransform->GetChildByIndex(0)->GetGameObject())
			{
				if (auto text = superChildObject->GetComponent<Engine::TextComponent>())
				{
					myLeaderboard.push_back(text);
				}
			}
		}
	}
}

void Leaderboard::Reflect(Engine::Reflector& aReflector)
{
	int size = (int)myLeaderboard.size();
	aReflector.Reflect(size, "Size");
}

void Leaderboard::ApplyFromLeaderboardManager()
{
	auto gm = GameManager::GetInstance();
	float difficulty = gm->GetGameSettings().myDifficultyMultiplier;

	auto& lb = gm->GetLeaderboard();
	lb.Apply();

	if (difficulty == 0.1f)
	{
		ApplyInternal(lb.myEducatorsMode);
	}
	else if (difficulty == 0.75f)
	{
		ApplyInternal(lb.myBeginner);
	}
	else if (difficulty == 1.0f)
	{
		ApplyInternal(lb.myNormal);
	}
	else if (difficulty == 1.5f)
	{
		ApplyInternal(lb.myHard);
	}
	else if (difficulty == 2.0f)
	{
		ApplyInternal(lb.myNightmare);
	}
	else if (difficulty == 4.0f)
	{
		ApplyInternal(lb.myImpossible);
	}
	else if (difficulty == 8.0f)
	{
		ApplyInternal(lb.myStarMode);
	}
	else
	{
		LOG_ERROR(LogType::Game) << "No difficulty multiplier matched that value";
		return;
	}
}

Engine::TextComponent* Leaderboard::GetScoreFromLeaderboard(const int anIndex)
{
	if (anIndex < myLeaderboard.size())
	{
		return myLeaderboard[anIndex];
	}
	return nullptr;
}

void Leaderboard::ApplyInternal(std::array<LeaderboardEntry, LEADERBOARD_COUNT>& anEntry)
{
	for (int i = 0; i < myLeaderboard.size(); i++)
	{
		std::string time = "--:--:--";
		auto& entry = anEntry[i];
		
		if (myLeaderboard[i] && entry.myTotalTime < FLT_MAX * 0.9f)
		{
			time = "";

			if (entry.myMinutes < 10)
				time += "0";
			time += std::to_string((int)entry.myMinutes);
			time += ":";

			if (entry.mySeconds < 10)
				time += "0";
			time += std::to_string((int)entry.mySeconds);
			time += ":";

			if (entry.myMilliSeconds < 10)
				time += "0";
			time += std::to_string((int)entry.myMilliSeconds);
		}

		myLeaderboard[i]->SetText(time);
	}
}
