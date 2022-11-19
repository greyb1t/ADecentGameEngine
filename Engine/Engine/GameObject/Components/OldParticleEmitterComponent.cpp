#include "pch.h"
#include "OldParticleEmitterComponent.h"

#include "Engine/GameObject/GameObject.h"

OldParticleEmitterComponent::OldParticleEmitterComponent(GameObject* aGameObject)
	: Component(aGameObject)
{
//	myEmitter = DBG_NEW VFXParticleEmitter();
}

void OldParticleEmitterComponent::Start()
{
//	myEmitter->SetDuration(4.f);
	LOG_INFO(LogType::Viktor) << "OldParticleEmitterComponent Start";
}

void OldParticleEmitterComponent::Execute(Engine::eEngineOrder aOrder)
{
//	myEmitter->Tick();

	//if (myEmitter->IsFinished())
	{
		LOG_INFO(LogType::Viktor) << "Particle Emitter object destroy";
		myGameObject->Destroy();
	}
}

//VFXParticleEmitter& OldParticleEmitterComponent::GetEmitter()
//{
//	//return *myEmitter;
//}
