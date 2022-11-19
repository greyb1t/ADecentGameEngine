#include "pch.h"
#include "VFXComponent.h"

#include "ParticleEmitterComponent.h"
#include "PhysicsObjectEmitterComponent.h"
#include "Engine/GameObject/GameObject.h"

Engine::VFXComponent::VFXComponent(GameObject* aGameObject, const VFX::VFXDescription& aDescription)
	: Component(aGameObject), myVFXSystem(aDescription)
{
	myVFXSystem.Subscribe([&](const VFX::ParticleEmitterDescription& description)
		{
			CreateEmitter(description);
		});
	myVFXSystem.Subscribe([&](const VFX::PrefabEmitterDescription& description)
		{
			CreateEmitter(description);
		});
}

void Engine::VFXComponent::Start()
{
}

void Engine::VFXComponent::Execute(eEngineOrder aOrder)
{
 	 myVFXSystem.Tick();
	 if (myCustomDuration > 0.f)
	 {
		 myCustomDuration -= Time::DeltaTime;
		 if (myCustomDuration <= 0.f)
		 {
			 Stop();
		 }
	 }
	 if (myIsAutoDestroy && myVFXSystem.GetState() == eVFXState::STOP && myVFXSystem.AutoDestroyCheck())
	 {
		myVFXSystem.AutoDestroyCheck();
		myGameObject->Destroy();
	 }
}

void Engine::VFXComponent::Play()
{
	myVFXSystem.Play();
}

void Engine::VFXComponent::Pause()
{
	myVFXSystem.Pause();
}

void Engine::VFXComponent::Stop()
{
	myVFXSystem.Stop();
}

void Engine::VFXComponent::ForceStop()
{
	myVFXSystem.ForceStop();
}

void Engine::VFXComponent::SetDuration(float aDuration)
{
	myCustomDuration = aDuration;
}

void Engine::VFXComponent::AutoDestroy(bool aAutoDestroy)
{
	myIsAutoDestroy = aAutoDestroy;
}

VFX::VFXSystem& Engine::VFXComponent::GetVFX()
{
	return myVFXSystem;
}

const bool Engine::VFXComponent::IsPlaying()
{
	return myVFXSystem.GetState() == eVFXState::PLAY;
}

void Engine::VFXComponent::CreateEmitter(const VFX::ParticleEmitterDescription& aDescription)
{
	auto obj = myGameObject->GetScene()->AddGameObject<GameObject>();


	if (myVFXSystem.GetDescription().space == eSpace::LOCALSPACE)
	{
		obj->GetTransform().SetParent(&myGameObject->GetTransform());
		obj->GetTransform().SetPositionLocal(aDescription.base.position);
		obj->GetTransform().SetRotationLocal(Quatf());
	}
	else 
	{
		const Vec3f descPos = aDescription.base.position;
		Vec3f pos = descPos.z * myGameObject->GetTransform().Forward() + descPos.x * myGameObject->GetTransform().Right() + descPos.y * myGameObject->GetTransform().Up();
		obj->GetTransform().SetPosition(myGameObject->GetTransform().GetPosition() + pos);
	}

	auto* emitter = obj->AddComponent<ParticleEmitterComponent>(aDescription);
	emitter->AttachVFX(&myVFXSystem);
}

void Engine::VFXComponent::CreateEmitter(const VFX::PrefabEmitterDescription& aDescription)
{
	auto obj = myGameObject->GetScene()->AddGameObject<GameObject>();

	if (aDescription.base.space == eSpace::LOCALSPACE)
	{
		obj->GetTransform().SetParent(&myGameObject->GetTransform());
		obj->GetTransform().SetPositionLocal({ 0,0,0 });
		obj->GetTransform().SetRotationLocal(Quatf());
	}


	//obj->AddComponent<PhysicsObjectEmitterComponent>(aDescription);
}
