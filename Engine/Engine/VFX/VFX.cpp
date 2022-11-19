#include "pch.h"
#include "VFX.h"
#include "Event/VFXEvent.h"
#include "Engine/GameObject/GameObject.h"

VFX::VFX(Weak<GameObject> aGameObject, const std::string& aVFXName)
{
	Init(aGameObject, aVFXName);
}

VFX::~VFX()
{
	for (int i = myEvents.size() - 1; i >= 0; i--) 
	{
		delete myEvents[i];
		myEvents[i] = nullptr;
	}
}

void VFX::Init(Weak<GameObject> aGameObject, const std::string& aVFXName)
{
	myVFXGameObject = aGameObject;
}

void VFX::Tick()
{
	if (myState != eVFXState::PLAY)
		return;

	myTimer += Time::DeltaTime;

	if (myTimer > myDuration)
	{
		Stop();
		return;
	}

	for (int i = 0; i < myEventTimers.size(); i++)
	{
		if (myEventTimers[i].TickAction(Time::DeltaTime / myDuration))
		{
			LOG_INFO(LogType::Viktor) << "VFX Activate event";
			myEvents[i]->Activate();
		}
	}
}

void VFX::Play()
{
	LOG_INFO(LogType::Viktor) << "VFX Play";
	Reset();
	myState = eVFXState::PLAY;
}

void VFX::Pause()
{
	LOG_INFO(LogType::Viktor) << "VFX Pause";
	myState = eVFXState::PAUSE;
}

void VFX::Stop()
{
	LOG_INFO(LogType::Viktor) << "VFX Stop";
	myState = eVFXState::STOP;
}

void VFX::Reset()
{
	myTimer = 0;
	for (auto& et : myEventTimers)
	{
		et.Reset();
	}
}

void VFX::AddEvent(TimeAction aTime, VFXEvent* aEvent)
{
	aEvent->SetVFX();
	myEvents.emplace_back(aEvent);
	myEventTimers.emplace_back(aTime);
}

void VFX::SetDuration(float aDuration)
{
	myDuration = aDuration;
}

GameObject* VFX::GetGameObject() const
{
	if (myVFXGameObject.expired())
		return nullptr;

	return myVFXGameObject.lock().get();
}

Transform& VFX::GetTransform()
{
	return myVFXGameObject.lock()->GetTransform();
}
