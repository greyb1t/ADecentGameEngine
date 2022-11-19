#pragma once
#include "Engine/GameObject/Transform.h"
#include "Time/TimeAction.h"
class VFXEvent;

class VFX
{
	enum class eVFXState
	{
		PLAY,
		PAUSE,
		STOP,
	};
public:
	VFX() = default;
	VFX(Weak<GameObject> aGameObject, const std::string& aVFXName);
	~VFX();
	void Init(Weak<GameObject> aGameObject, const std::string& aVFXName);

	void Tick();

	void Play();
	void Pause();
	void Stop();

	void Reset();

	void AddEvent(TimeAction aTime, VFXEvent* aEvent);

	void SetDuration(float aDuration);
	
	GameObject* GetGameObject() const;
	Transform& GetTransform();
protected:
	Weak<GameObject> myVFXGameObject;

	std::vector<VFXEvent*> myEvents;
	std::vector<TimeAction> myEventTimers;

	eVFXState myState = eVFXState::STOP;


	std::string myName;

	float myDuration = 1;
	
	float myTimer = 0;
};

