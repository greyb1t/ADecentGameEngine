#include "pch.h"
#include <Engine/GameObject/Components/ScriptsComponent.h>
#include "AudioComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/Reflection/Reflector.h"
#include "Engine/AudioManager.h"

Engine::AudioComponent::AudioComponent(GameObject* aGameObject)
	: Component(aGameObject)
{

}

Engine::AudioComponent::~AudioComponent()
{
	AudioManager::GetInstance()->Stop(myGameObject->GetUUID());
	ReleaseAudio();
}
void Engine::AudioComponent::Start()
{
	AddEvent("Idle", myIdleSoundPath);
	myIdleSoundTime = Random::RandomFloat(myIdleSoundTimeMin, myIdleSoundTimeMax);
}
void Engine::AudioComponent::Execute(eEngineOrder aOrder)
{
	PlayIdleSound();
}
void Engine::AudioComponent::StopIdle()
{
	myShouldPlay = false;
}
void Engine::AudioComponent::PlayIdle()
{
	myShouldPlay = true;
}
void Engine::AudioComponent::PlayIdleSound()
{
	if (myShouldPlay)
	{
		if (myIdleLoop)
		{
			myIdleSoundTimer = 0;
			PlayEvent("Idle");
			myShouldPlay = false;
		}
		if (myIdleSoundPath != "NONE")
		{
			if (myIdleSoundTime > 0.0f)
			{
				myIdleSoundTimer += Time::DeltaTime;

				if (myIdleSoundTimer >= myIdleSoundTime)
				{
					PlayEvent("Idle");
					myIdleSoundTimer = 0.0f;
				}
			}
			else
			{
				myIdleSoundTime = Random::RandomFloat(myIdleSoundTimeMin, myIdleSoundTimeMax);
			}
		}
	}
}

void Engine::AudioComponent::Reflect(Reflector& aReflector)
{
	//Must be in all components
	Component::Reflect(aReflector);
	aReflector.Reflect(myVolume, "Sound Volume");
	aReflector.Reflect(myIdleLoop, "FMOD Looping");
	aReflector.Reflect(myIdleSoundTimeMin, "Idle Sound Cooldown Min");
	aReflector.Reflect(myIdleSoundTimeMax, "Idle Sound Cooldown Max");
	auto result = aReflector.Reflect(myIdleSoundPath, "Idle Sound Path");
	if (result == ReflectorResult::ReflectorResult_Changed)
	{
		AddEvent("Idle", myIdleSoundPath);
	}
	aReflector.Reflect(myEventNames, "Events In Component");
}
void Engine::AudioComponent::ReleaseAudio()
{
	AudioManager::GetInstance()->ReleaseAudio(myGameObject->GetUUID());
}
//Play With Path
//void Engine::AudioComponent::PlayEvent(std::string aEventPath)
//{
//	auto trans = myGameObject->GetTransform();
//	AudioManager::GetInstance()->PlayEvent3D(aEventPath, myGameObject->GetUUID(), trans.GetPosition(), trans.Forward(), trans.Up());
//}
//Play With Event Name
void Engine::AudioComponent::PlayEvent(const std::string& aEventName)
{
	if (this == nullptr) {
		LOG_ERROR(LogType::Audio) << "AudioComponent is null";
	}
	auto& trans = myGameObject->GetTransform();

	if (!myEvents.empty())
	{
		for (AudioEvent& event : myEvents)
		{
			if (!event.myEventName.empty())
			{
				if (event.myEventName == aEventName)
				{
					AudioManager::GetInstance()->PlayEvent3D(event.myEventPath, myGameObject->GetUUID(), trans.GetPosition(), trans.Forward(), trans.Up());
					break;
				}
			}
		}
	}
	else 
	{
		LOG_ERROR(LogType::Audio) << "myEvents is empty";
	}
}
bool Engine::AudioComponent::IsInstanceLoaded(const std::string& aPath)
{
	return AudioManager::GetInstance()->IsInstanceLoaded(myGameObject->GetUUID(), aPath);
}

void Engine::AudioComponent::ChangeParameter(const std::string& aParameterName, float aValue)
{
	AudioManager::GetInstance()->ChangeParameter(aParameterName, aValue);
}

void Engine::AudioComponent::ChangeParameter(int aUUID, const std::string& aEventPath, const std::string& aParameterName, float aValue)
{
	AudioManager::GetInstance()->ChangeParameter(aUUID, aEventPath, aParameterName, aValue);
}
void Engine::AudioComponent::SetIdleSoundTimes(float aMin, float aMax)
{
	myIdleSoundTimeMin = aMin;
	myIdleSoundTimeMax = aMax;

	myIdleSoundTime = Random::RandomFloat(myIdleSoundTimeMin, myIdleSoundTimeMax);
}
void Engine::AudioComponent::SetIdleSoundPath(const std::string& aPath)
{
	myIdleSoundPath = aPath;
}
void Engine::AudioComponent::AddEvent(const std::string& aEventName, const std::string& aEventPath)
{
	AudioEvent event;
	event.myEventName = aEventName;
	event.myEventPath = aEventPath;
	myEvents.push_back(event);

	myEventNames.push_back(aEventName);
	AudioManager::GetInstance()->LoadEvent(myGameObject->GetUUID(), aEventPath);
}

void Engine::AudioComponent::PauseAll()
{

}

void Engine::AudioComponent::PauseEvent(const std::string& aEventPath)
{

}

void Engine::AudioComponent::StopAll()
{
	AudioManager::GetInstance()->Stop(myGameObject->GetUUID());
}

void Engine::AudioComponent::StopEvent(const std::string& aEvent)
{
	const std::string& eventPath = GetEventPath(aEvent);

	AudioManager::GetInstance()->Stop(eventPath, myGameObject->GetUUID());
}
const std::string& Engine::AudioComponent::GetEventPath(const std::string& aEvent)
{
	const std::string& eventPath = "0";
	for (auto& audioInfo : myEvents)
	{
		if (audioInfo.myEventName == aEvent)
		{
			return audioInfo.myEventPath;
		}
	}

	if (eventPath == "0")
	{
		LOG_ERROR(LogType::Audio) << "Event has no path or does not exist";
		return "NONE";
	}

	// NOTE(filip): lade till denna för att du returnade inget om ovan ifs inte körde
	const static std::string empty = "money";
	return empty;
}
