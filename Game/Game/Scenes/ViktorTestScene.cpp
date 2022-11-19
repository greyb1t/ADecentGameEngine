#include "pch.h"
#include "ViktorTestScene.h"

#include <Components/Interactable.h>

#include "Engine/GameObject/GameObject.h"
#include "Game/Components/DestructibleComponent.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/ModelComponent.h"
#include "Engine/GameObject/Components/RigidBodyComponent.h"
#include "Engine/GameObject/Components/RigidStaticComponent.h"
#include "Engine/GameObject/Components/InfoRigidBodyComponent.h"

#include "Physics/Physics.h"
#include "Engine/Shortcuts.h"
#include "Physics/CookingManager.h"
#include <Engine/GameObject/Components/VFXComponent.h>
#include <Game/Components/HealthComponent.h>

#include "Common/BinaryReader.h"
#include "Components/HealthComponent.h"
#include "Engine/Renderer/Model/Model.h"
#include "Engine/Renderer/ModelLoader/FbxLoader.h"
#include "Engine/Renderer/ModelLoader/Binary/model_generated.h"
#include "Engine/VFX/Time/Timeline.h"
#include "Engine/VFX/Time/TimeAction.h"
#include "Engine/Utils/PhysicsUtils.h"
#include "Engine/VFX/OldParticleSystem.h"
#include "Prototype/Dummy.h"
#include "Prototype/RorPlayer.h"
//#include "Engine/VFX/Event/PhysicsObjectEmitterEvent.h"
//#include <Engine/VFX/Event/ParticleEmitterEvent.h>
//#include "Engine/VFX/Emitter/PhysicsObjectEmitter.h"

Engine::Scene::Result ViktorTestScene::OnRuntimeInit()
{
	FolderScene::OnRuntimeInit();
	
	{
		auto interactableObj = AddGameObject<GameObject>();
		interactableObj->GetTransform().SetPosition({ 0,700,300 });

		interactableObj->SetLayer(eLayer::DEFAULT);
		interactableObj->SetPhysicalLayer(eLayer::GROUND | eLayer::DEFAULT);

		auto rb = interactableObj->AddComponent<Engine::RigidBodyComponent>();
		rb->Attach(Shape::Box(Vec3f(200, 100, 100)));
		static Engine::RigidBodyComponent* srb;
		srb = rb;

		auto interactable = interactableObj->AddComponent<Interactable>();
		interactable->ObserveInteract([&]()
			{
				srb->SetVelocity({0,500,0});
			});
	}
	if (false)
	{
		auto ground = AddGameObject<GameObject>();
		ground->GetTransform().SetPosition({ 0,-50,0 });

		ground->SetLayer(eLayer::GROUND);
		//ground->SetPhysicalLayer(eLayer::GROUND);
		auto rb = ground->AddComponent<Engine::RigidStaticComponent>();
		rb->Attach(Shape::Box(Vec3f(5000, 100, 5000)));
	}

	if (false)
	{
		auto trigger = AddGameObject<GameObject>();
		trigger->GetTransform().SetPosition({ 400,1000,300 });
		trigger->SetLayers(eLayer::ZONE, eLayer::NONE, eLayer::ALL);


		trigger->SetPhysicalLayer(eLayer::GROUND);
		trigger->SetDetectionLayer(eLayer::ALL);
		auto rb = trigger->AddComponent<Engine::RigidBodyComponent>();
		rb->Attach(Shape::Box(Vec3f(300, 222, 300)));
	}

	{
		auto trigger = AddGameObject<GameObject>();
		trigger->GetTransform().SetPosition({ 400,200,300 });
		trigger->SetLayers(eLayer::ZONE, eLayer::NONE, eLayer::PLAYER_DAMAGEABLE);


		//trigger->SetPhysicalLayer(eLayer::GROUND);
		auto rb = trigger->AddComponent<Engine::RigidBodyComponent>(eRigidBodyType::TRIGGER);
		rb->Attach(Shape::Box(Vec3f(300, 300, 300)));
		rb->SetCollisionListener(true);

		rb->ObserveCollision([](GameObject* obj)
			{
				LOG_INFO(LogType::Viktor) << "Trigger enter";
			});
		rb->ObserveTriggerEnter([](GameObject* obj)
			{
				LOG_INFO(LogType::Viktor) << "Trigger stay";
			});
		rb->ObserveTriggerExit([](GameObject* obj)
			{
				LOG_INFO(LogType::Viktor) << "Trigger exit";
			});
	}



	{
		auto enemy = AddGameObject<GameObject>();
		enemy->SetLayer(eLayer::ENEMY | eLayer::DAMAGEABLE);
		enemy->SetPhysicalLayer(eLayer::GROUND);
		auto hp = enemy->AddComponent<HealthComponent>(100);
		hp->ObserveDeath([](float dmg)
		{
			LOG_INFO(LogType::Viktor) << "Enemy killed!";
		});

		auto rb = enemy->AddComponent<Engine::RigidBodyComponent>(); 
		rb->Attach(Shape::Sphere(100));

		hp->ObserveDamage([&rb](float dmg)
		{
				LOG_INFO(LogType::Viktor) << "Dmg: " << dmg;
		});
		enemy->GetTransform().SetPosition({ 0,0,0 });
	}

	return Engine::Scene::Result::Succeeded;
}

ViktorTestScene* ViktorTestScene::Clone() const
{
	return DBG_NEW ViktorTestScene(*this);
}

void ViktorTestScene::Update(TimeStamp ts)
{
	FolderScene::Update(ts);

	RayCastHit hit;
	if (false)// (GetEngine().GetInputManager().IsMouseKeyDown(Common::MouseButton::Left))
	{
		if (RayCast(GetMainCamera().GetTransform().GetPosition(),  ScreenToWorldDirection(GetEngine().GetInputManager().GetMousePosition()), 100000, eLayer::DAMAGEABLE | eLayer::GROUND, hit))
		{
			LOG_INFO(LogType::Viktor) << "Layer: " << hit.Layer;
			if (hit.Layer & eLayer::DAMAGEABLE)
			{
				auto dir = (hit.Position - hit.Origin).GetNormalized() * 100.f;

				auto hp = hit.GameObject->GetComponent<HealthComponent>();
				hp->ApplyDamage(33.f);

				if (auto* rb = hit.GameObject->GetComponent<Engine::RigidBodyComponent>())
				{
					rb->SetVelocity(Vec3f(0,400,0) + Random::InsideUnitSphere() * 200.f);
				}
			}

			GDebugDrawer->DrawCircle3D(DebugDrawFlags::Always, hit.Position, 20, 3, Vec4f(1, .7f, .7f, .7f));
		}
	}
}
