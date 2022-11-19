#pragma once
#include "Description/VFXDescription.h"
#include "Engine/GameObject/Transform.h"
#include "Description/ParticleEmitterDescription.h"
#include "Description/PhysicsEmitterDescription.h"
#include "Description/PhysicsParticleEmitterDescription.h"
#include <functional>

#include "VFXEnums.h"

namespace VFX
{
	class VFXChild;

	class VFXSystem
{
public:
	VFXSystem() = default;
	VFXSystem(const VFXDescription& aDescription);
	~VFXSystem();
	void Init(const VFXDescription& aDescription);

	void Tick();

	void Play();
	void Pause();
	void Stop();
	void ForceStop();

	void Reset();

	void Subscribe(const std::function<void(const ParticleEmitterDescription&)>& subscription);
	void Subscribe(const std::function<void(const PrefabEmitterDescription&)>& subscription);
	
	void SetDuration(float aDuration);

	void Attach(VFXChild* aChild);
	void Detach(VFXChild* aChild);

	bool AutoDestroyCheck();

	VFXDescription& GetDescription();
	Transform&		GetTransform();
	eVFXState&		GetState();
protected:
	Transform myTransform;
	VFXDescription myDescription;
	
	eVFXState myState = eVFXState::STOP;
	float myTimer = 0;

	std::function<void(const ParticleEmitterDescription&)> myParticleEmitterSubscription;
	std::function<void(const PrefabEmitterDescription&)> myPrefabEmitterSubscription;

	std::vector<VFXChild*> myChildren;
	bool myIsInited = false;
};
}
