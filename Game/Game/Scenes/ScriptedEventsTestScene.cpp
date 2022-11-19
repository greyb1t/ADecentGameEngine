#include "pch.h"
#include "ScriptedEventsTestScene.h"

#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/ModelComponent.h"
#include <Engine/TriggerEventManager/TriggerEventManager.h>
#include <Physics/RigidBody.h>
#include <Engine/GameObject/Components/RigidBodyComponent.h>

Engine::Scene::Result ScriptedEventsTestScene::OnRuntimeInit()
{
	FolderScene::OnRuntimeInit();

	myGameObject = AddGameObject<GameObject>();
	myGameObject->GetTransform().SetPosition({ 0.f ,0.f, 0.f });
	myGameObject->SetName("TEST");

	auto rs = myGameObject->AddComponent<Engine::RigidBodyComponent>(eRigidBodyType::KINEMATIC);
	rs->Attach(Shape::Box({7000.f, 1.f, 7000.f}));
	

	return Engine::Scene::Result::Succeeded;
}

ScriptedEventsTestScene* ScriptedEventsTestScene::Clone() const
{
	return DBG_NEW ScriptedEventsTestScene(*this);
}

void ScriptedEventsTestScene::Update(TimeStamp ts)
{

	FolderScene::Update(ts);
}
