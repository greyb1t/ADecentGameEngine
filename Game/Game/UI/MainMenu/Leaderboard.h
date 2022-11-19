#pragma once

#include "Engine/GameObject/Components/Component.h"

class Leaderboard : public Component
{
	COMPONENT(Leaderboard, "Leader Board");
public:
	Leaderboard() = default;
	~Leaderboard() = default;

	void Awake() override;

	void Reflect(Engine::Reflector& aReflector) override;

	void ApplyFromLeaderboardManager();

	Engine::TextComponent* GetScoreFromLeaderboard(const int anIndex);

private:
	void ApplyInternal(std::array<LeaderboardEntry, LEADERBOARD_COUNT>& anEntry);

private:
	std::vector<Engine::TextComponent*> myLeaderboard;

};