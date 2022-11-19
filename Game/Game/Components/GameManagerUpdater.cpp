#include "pch.h"
#include "GameManagerUpdater.h"

#include "Game/GameManager/GameManager.h"

void GameManagerUpdater::Start()
{
	GameManager::GetInstance()->Resume();
}

void GameManagerUpdater::Execute(Engine::eEngineOrder)
{
	GameManager::GetInstance()->Update();
}