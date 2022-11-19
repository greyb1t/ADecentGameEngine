#include "pch.h"
#include "PhysicsObjectEmitterComponent.h"

#include "RigidBodyComponent.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/Scene/SceneManager.h"
//#include "Engine/VFX/Description/EmitterDescription.h"
//#include "Engine/VFX/Emitter/PhysicsObjectEmitter.h"

PhysicsObjectEmitterComponent::PhysicsObjectEmitterComponent(GameObject* aGameObject, const VFX::PhysicsEmitterDescription& aDescription)
	: Component(aGameObject), myDescription(aDescription), VFX::Emitter(aDescription.base)
{
}

void PhysicsObjectEmitterComponent::Start()
{
}

void PhysicsObjectEmitterComponent::Execute(Engine::eEngineOrder aOrder)
{
	VFX::Emitter::Tick();
	if (VFX::Emitter::IsFinished())
	{
		LOG_INFO(LogType::Viktor) << "PhysicsComponent Emitter object destroy";
		myGameObject->Destroy();
	}
}

void PhysicsObjectEmitterComponent::Emit()
{
	LOG_INFO(LogType::Viktor) << "Emit!";

	auto* obj = myGameObject->GetScene()->AddGameObject<GameObject>();

	auto emitSpawn = myDescription.base.geometry.Get();
	obj->GetTransform().SetPosition(myGameObject->GetTransform().GetPosition() + emitSpawn.position);
	obj->GetTransform().SetRotation(myGameObject->GetTransform().GetRotation());

	auto* rb = obj->AddComponent<Engine::RigidBodyComponent>(eRigidBodyType::DEFAULT);

	rb->Attach(Shape::Sphere(25.f));

	rb->AddForce(emitSpawn.rotation.ToMatrix().GetForward() * 1000.f, eForceMode::IMPULSE);
}

//PhysicsObjectEmitter& PhysicsObjectEmitterComponent::GetEmitter()
//{
//	return *myEmitter;
//}
