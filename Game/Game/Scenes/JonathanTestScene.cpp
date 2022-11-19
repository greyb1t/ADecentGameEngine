#include "pch.h"
#include "JonathanTestScene.h"

#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/RigidBodyComponent.h"
#include "Engine/GameObject/Components/RigidStaticComponent.h"
#include "Physics/CookingManager.h"
#include "Camera/CameraController.h"

Engine::Scene::Result JonathanTestScene::OnRuntimeInit()
{
	FolderScene::OnRuntimeInit();

	{
		myGameObject = AddGameObject<GameObject>();
		myGameObject->GetTransform().SetPosition({ 0, -500, 0 });

		auto rs = myGameObject->AddComponent<Engine::RigidStaticComponent>();

		auto box = Shape::Box({ 1000, 100, 1000 });
		rs->Attach(box);
		myGameObject->SetLayers(eLayer::DEFAULT, eLayer::DEFAULT, eLayer::DEFAULT);
		//auto model = go->AddComponent<Engine::ModelComponent>(
		//	"Assets/Meshes/BasicShapes/Cube/EN_G_Cube_01.model");
	}

	{
		auto go = AddGameObject<GameObject>();
		go->GetTransform().SetPosition({ 0, 500, 0 });

		auto rs = go->AddComponent<Engine::RigidBodyComponent>();

		auto box = Shape::Box({ 100, 100, 100 });
		box.SetLocalPosition({ 100,100, 0 });
		auto box2 = Shape::Box({ 100, 100, 100 });
		box2.SetLocalPosition({ 0,100, 300 });
		//rs->SetOffset({ 100,100,100 });
		rs->Attach(box);
		rs->Attach(box2);
		go->SetLayers(eLayer::DEFAULT, eLayer::DEFAULT, eLayer::DEFAULT);
	}


	return Engine::Scene::Result::Succeeded;
}

JonathanTestScene* JonathanTestScene::Clone() const
{
	return DBG_NEW JonathanTestScene(*this);
}

void JonathanTestScene::Update(TimeStamp ts)
{
	FolderScene::Update(ts);
}
