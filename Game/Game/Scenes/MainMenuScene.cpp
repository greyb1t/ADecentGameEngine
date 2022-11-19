#include "pch.h"
#include "MainMenuScene.h"

#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/ModelComponent.h"

Engine::Scene::Result MainMenuScene::OnRuntimeInit()
{
	FolderScene::OnRuntimeInit();

	return Engine::Scene::Result::Succeeded;
}

MainMenuScene* MainMenuScene::Clone() const
{
	return DBG_NEW MainMenuScene(*this);
}

void MainMenuScene::Update(TimeStamp ts)
{
	FolderScene::Update(ts);

}
