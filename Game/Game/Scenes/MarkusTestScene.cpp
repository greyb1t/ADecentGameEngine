#include "pch.h"
#include "MarkusTestScene.h"

#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/ModelComponent.h"

Engine::Scene::Result MarkusTestScene::OnRuntimeInit()
{
	FolderScene::OnRuntimeInit();

	return Engine::Scene::Result::Succeeded;
}

MarkusTestScene* MarkusTestScene::Clone() const
{
	return DBG_NEW MarkusTestScene(*this);
}

void MarkusTestScene::Update(TimeStamp ts)
{
	FolderScene::Update(ts);
}
