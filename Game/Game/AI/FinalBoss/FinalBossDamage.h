#pragma once

#include "Game/GameManager/GameManager.h"

namespace FB
{
	class FinalBossDamage : public Engine::Reflectable
	{
	public:
		void Reflect(Engine::Reflector& aReflector) override;

		float GetDamage() const;

	private:
		std::array<Vec2f, static_cast<int>(GameDifficulty::_size())> myTierList;
	};
}