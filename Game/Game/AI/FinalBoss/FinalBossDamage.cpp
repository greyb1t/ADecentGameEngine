#include "pch.h"
#include "FinalBossDamage.h"

void FB::FinalBossDamage::Reflect(Engine::Reflector& aReflector)
{
	for (int i = 0; i < GameDifficulty::_size(); ++i)
	{
		aReflector.SetNextItemSpeed(0.1f);
		aReflector.Reflect(myTierList[i], GameDifficulty::_from_integral(i)._to_string() + std::string(" Min Max"));

		if (myTierList[i].y < myTierList[i].x)
		{
			myTierList[i].y = myTierList[i].x;
		}
	}
}

float FB::FinalBossDamage::GetDamage() const
{
	if (auto gm = GameManager::GetInstance())
	{
		GameDifficulty difficulty = gm->GetGameDifficulty();
		const auto& dmgMinMax = myTierList[difficulty];

		const float difficultyFactor = GameManager::GetInstance()->GetGameSettings().myDifficultyMultiplier;

		return Random::RandomFloat(dmgMinMax.x, dmgMinMax.y) * difficultyFactor;
	}

	LOG_ERROR(LogType::Game) << "Missing GameManager when getting final boss damage";

	return 0.f;
}
