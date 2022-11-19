#include "pch.h"
#include "SimonTestScene.h"

#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/ModelComponent.h"

#include "Camera/CameraController.h"

#include "Engine/GameObject/Components/ModelComponent.h"
#include "Engine/GameObject/Components/RigidBodyComponent.h"

#include <Engine\TriggerEventManager\TriggerEventManager.h>


Engine::Scene::Result SimonTestScene::OnRuntimeInit()
{
	FolderScene::OnRuntimeInit();
	

	{//Vägg
		auto cube = AddGameObject<GameObject>();

		auto rb = cube->AddComponent<Engine::RigidBodyComponent>(eRigidBodyType::KINEMATIC);
		rb->Attach(Shape::Box({ 500.f, 500.f, 100.f }));

		cube->GetTransform().SetPosition({ 0.0f, 200.0f, 0.0f });
	}


	{//Ramp
		auto cube = AddGameObject<GameObject>();

		auto rb = cube->AddComponent<Engine::RigidBodyComponent>(eRigidBodyType::KINEMATIC);
		rb->Attach(Shape::Box({ 500.f, 1000.f, 100.f }));

		cube->GetTransform().SetPosition({ 1000.0f, 200.0f, 0.0f });
		cube->GetTransform().SetRotation(Vec3f(45.0f, 0.0f, 0.0f));
	}



	{//Flygande kub
		auto cube = AddGameObject<GameObject>();

		auto rb = cube->AddComponent<Engine::RigidBodyComponent>(eRigidBodyType::KINEMATIC);
		rb->Attach(Shape::Box({ 500.f, 500.f, 500.f }));

		cube->GetTransform().SetPosition({ 0.0f, 500.0f, 0.0f });
	}


	{//Golv
		auto cube = AddGameObject<GameObject>();

		auto rb = cube->AddComponent<Engine::RigidBodyComponent>(eRigidBodyType::KINEMATIC);
		rb->Attach(Shape::Box({ 10000.f, 10.f, 10000.f }));

		cube->GetTransform().SetPosition({ 0.0f, -30.0f, 0.0f });
	}


	{
		//auto gameObject = AddGameObject<GameObject>();

		//Vec3f size(500.f, 500.f, 500.f);

		//auto shape = Shape::Box(size);
		//gameObject->SetLayers(eLayer::NONE, eLayer::NONE, eLayer::NONE);

		//auto& objectTransform = gameObject->GetTransform();
		//auto* rb = new Engine::RigidBodyComponent();

		//rb = gameObject->AddComponent<Engine::RigidBodyComponent>(eRigidBodyType::TRIGGER);

		////auto rb =  new Engine::RigidBodyComponent(gameObject);

		//rb->SetLocalRotation(gameObject->GetTransform().GetRotationLocal());

		//physx::PxTransform myPxTransform;
		//myPxTransform.p = { objectTransform.GetPosition().x, objectTransform.GetPosition().y, objectTransform.GetPosition().z };
		//myPxTransform.q = physx::PxQuat(objectTransform.GetRotation().myVector.x,
		//	objectTransform.GetRotation().myVector.y,
		//	objectTransform.GetRotation().myVector.z,
		//	objectTransform.GetRotation().myW);
		//rb->GetActor()->setGlobalPose(myPxTransform);

		////rb->SetOffset(myOffset);
		////rb->SetMass(myMass);

		//rb->Attach(Shape::Copy(shape));

		//rb->ObserveTriggerEnter([&](GameObject* obj)
		//{
		//	TriggerEventManager::GetInstance().SendLetter("WhiteBoardEvent", obj);
		//});
	}


	return Engine::Scene::Result::Succeeded;
}

SimonTestScene* SimonTestScene::Clone() const
{
	return DBG_NEW SimonTestScene(*this);
}

void SimonTestScene::Update(TimeStamp ts)
{
	FolderScene::Update(ts);
}
