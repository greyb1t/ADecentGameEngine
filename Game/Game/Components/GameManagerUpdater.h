#pragma once

#include "Engine/GameObject/Components/Component.h"

class GameManagerUpdater final : public Component
{
public:
	COMPONENT(GameManagerUpdater, "GameManagerUpdater");

	GameManagerUpdater() = default;

	void Start() override;
	void Execute(Engine::eEngineOrder) override;
};