#include "pch.h"
#include "GameScene.h"

Engine::Scene::Result GameScene::OnRuntimeInit()
{
	FolderScene::OnRuntimeInit();

	return Engine::Scene::Result::Succeeded;
}

GameScene* GameScene::Clone() const
{
	return DBG_NEW GameScene(*this);
}

void GameScene::Update(TimeStamp ts)
{
	FolderScene::Update(ts);
}
