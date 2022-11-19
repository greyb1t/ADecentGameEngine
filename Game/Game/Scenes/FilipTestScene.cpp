#include "pch.h"
#include "FilipTestScene.h"

#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/ModelComponent.h"

Engine::Scene::Result FilipTestScene::OnRuntimeInit()
{
	FolderScene::OnRuntimeInit();

	myGameObject = AddGameObject<GameObject>();
	myGameObject->GetTransform().SetScale(Vec3f(1, 1, 1));
	auto model = myGameObject->AddComponent<Engine::ModelComponent>(
		"Assets/Meshes/BasicShapes/Cube/EN_G_Cube_01.model");

	// auto comp = myGameObject->AddComponent<TestComponent>();

	return Engine::Scene::Result::Succeeded;
}

FilipTestScene* FilipTestScene::Clone() const
{
	return DBG_NEW FilipTestScene(*this);
}

void FilipTestScene::Update(TimeStamp ts)
{
	FolderScene::Update(ts);

	// myGameObject->GetTransform().SetPosition(
	// 	Vec3f(0.f, sin(GetEngine().GetTimer().GetTotalTime()) * 100.f, 0.f));
}
