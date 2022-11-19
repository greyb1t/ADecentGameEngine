#include "pch.h"
#include "VFXSystem.h"
#include "Engine/GameObject/GameObject.h"
#include "VFXChild.h"

VFX::VFXSystem::VFXSystem(const VFX::VFXDescription& aDescription)
{
	Init(aDescription);
}

VFX::VFXSystem::~VFXSystem()
{
	for (int i = static_cast<int>(myChildren.size()) - 1; i >= 0; --i)
	{
		myChildren[i]->DetachVFX();
	}
}

void VFX::VFXSystem::Init(const VFX::VFXDescription& aDescription)
{
	myDescription = aDescription;

	myDescription.particleEmitters.Subscribe([&] (const ParticleEmitterDescription& aParticleEmitterDescription)
		{
			if (myParticleEmitterSubscription)
				myParticleEmitterSubscription(aParticleEmitterDescription);
		});

	myDescription.prefabEmitters.Subscribe([&](const PrefabEmitterDescription& aPhysicsEmitterDescription)
		{
			if (myPrefabEmitterSubscription)
				myPrefabEmitterSubscription(aPhysicsEmitterDescription);
		});
}

void VFX::VFXSystem::Tick()
{
	if (myState != eVFXState::PLAY)
		return;

	myTimer += Time::DeltaTime;

	if (myTimer > myDescription.duration)
	{
		if (myDescription.looping && myIsInited)
		{
			Play();
		}
		else {
			Stop();
			return;
		}
	}

	myDescription.particleEmitters.Tick(Time::DeltaTime / myDescription.duration);
	myDescription.prefabEmitters.Tick(Time::DeltaTime / myDescription.duration);

	//for (int i = 0; i < myEventTimers.size(); i++)
	//{
	//	if (myEventTimers[i].TickAction(Time::DeltaTime / myDuration))
	//	{
	//		LOG_INFO(LogType::Viktor) << "VFXSystem Activate event";
	//		myEvents[i]->Activate();
	//	}
	//}
}

void VFX::VFXSystem::Play()
{
	myIsInited = false;
	LOG_INFO(LogType::Particles) << "VFXSystem Play";
	Reset();
	myState = eVFXState::PLAY;

	Tick();
	myIsInited = true;
}

void VFX::VFXSystem::Pause()
{
	LOG_INFO(LogType::Particles) << "VFXSystem Pause";
	myState = eVFXState::PAUSE;
}

void VFX::VFXSystem::Stop()
{
	LOG_INFO(LogType::Particles) << "VFXSystem Stop";
	myState = eVFXState::STOP;

	for (auto* child : myChildren)
	{
		child->Stop();
	}
}

void VFX::VFXSystem::ForceStop()
{
	LOG_INFO(LogType::Particles) << "VFXSystem FORCE Stop";
	myState = eVFXState::STOP;

	for (auto* child : myChildren)
	{
		child->ForceStop();
	}
}

void VFX::VFXSystem::Reset()
{
	myTimer = 0;
	myDescription.particleEmitters.Reset();
	myDescription.prefabEmitters.Reset();
}

void VFX::VFXSystem::Subscribe(const std::function<void(const ParticleEmitterDescription&)>& subscription)
{
	myParticleEmitterSubscription = subscription;
}

void VFX::VFXSystem::Subscribe(const std::function<void(const PrefabEmitterDescription&)>& subscription)
{
	myPrefabEmitterSubscription = subscription;
}

void VFX::VFXSystem::SetDuration(float aDuration)
{
	myDescription.duration = aDuration;
}

void VFX::VFXSystem::Attach(VFXChild* aChild)
{
	if (!aChild)
		return;
	LOG_INFO(LogType::Viktor) << "Attach";

	myChildren.emplace_back(aChild);
}

void VFX::VFXSystem::Detach(VFXChild* aChild)
{
	if (!aChild)
		return;

	LOG_INFO(LogType::Viktor) << "Detach";

	for (int i = 0; i < myChildren.size(); i++)
	{
		if (myChildren[i] == aChild)
		{
			myChildren.erase(myChildren.begin() + i);
		}
	}
}

bool VFX::VFXSystem::AutoDestroyCheck()
{
	for (int i = 0; i < myChildren.size(); i++)
	{
		if (!myChildren[i]->AutoDestroyCheck())
			return false;
	}
	return true;
}

VFX::VFXDescription& VFX::VFXSystem::GetDescription()
{
	return myDescription;
}

Transform& VFX::VFXSystem::GetTransform()
{
	return myTransform;
}

eVFXState& VFX::VFXSystem::GetState()
{
	return myState;
}
