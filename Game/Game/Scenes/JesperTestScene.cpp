#include "pch.h"
#include "JesperTestScene.h"

#include "Engine/GameObject/GameObject.h"
#include "Engine/Scene/SceneManager.h"

Engine::Scene::Result JesperTestScene::OnRuntimeInit()
{
	/*FolderScene::OnRuntimeInit();

	myGameObject = AddGameObject<GameObject>();
	myGameObject->GetTransform().SetPosition({ 3300.f ,0.f, 1700.f });
	myGameObject->SetName("TEST");*/
	//auto rs = myGameObject->AddComponent<Engine::RigidBodyComponent>(eRigidBodyType::KINEMATIC);
	//rs->Attach(Shape::Box({7000.f, 1.f, 7000.f}));


	//auto rb = myGameObject->AddComponent<Engine::RigidBodyComponent>();
	//rb->Attach(Shape::Box({ 10000.f, 10000.f, 10000.f }));

	//myGameObject->SetLayers(eLayer::GROUND, eLayer::ALL, eLayer::DEFAULT);

	//auto* rs = myGameObject->AddComponent<RigidStaticComponent>();


	//auto* rs = myGameObject->AddComponent<Engine::RigidBodyComponent>();
	//rs->SetType(eRigidBodyType::KINEMATIC);





	//myGameObject2 = AddGameObject<GameObject>();
	//myGameObject2->GetTransform().SetPosition({ 0.f ,0.f, 0.f });
	////myGameObject2->GetTransform().SetScale(Vec3f(1, 1, 1));

	//myGameObject2->SetLayers(eLayer::GROUND, eLayer::ALL, eLayer::DEFAULT);
	//auto* rb = myGameObject2->AddComponent<Engine::RigidBodyComponent>();
	//rb->SetType(eRigidBodyType::DEFAULT);
	//rb->Attach(Shape::Box({ 20000.f, 2000.f, 2000.f }));

	return Engine::Scene::Result::Succeeded;
}

JesperTestScene* JesperTestScene::Clone() const
{
	return DBG_NEW JesperTestScene(*this);
}

void JesperTestScene::Update(TimeStamp ts)
{
	ZoneScopedN("JesperTestScene::Update");

	FolderScene::Update(ts);
}


