#pragma once

#include "Engine/GameObject/Components/Component.h"

namespace FB
{
	class FinalBossArenaTrigger : public Component
	{
	public:
		COMPONENT(FinalBossArenaTrigger, "FB ArenaTrigger");

		void Start() override;

		void Execute(Engine::eEngineOrder aOrder) override;

		void Reflect(Engine::Reflector& aReflector) override;

	private:
		bool myHasPlayerEnteredTrigger = false;

	};
}