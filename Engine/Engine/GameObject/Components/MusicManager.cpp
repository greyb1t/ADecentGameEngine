#include "pch.h"
#include <Engine/GameObject/Components/ScriptsComponent.h>
#include <Engine/GameObject/Components/AudioComponent.h>
#include "MusicManager.h"
#include "Engine/Engine.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/Reflection/Reflector.h"
#include "Engine/AudioManager.h"

Engine::MusicManager::MusicManager(GameObject* aGameObject)
	: Component(aGameObject)
{

}

Engine::MusicManager::~MusicManager()
{
	if (myGameObject->GetComponent<AudioComponent>())
	{
		auto audioComp = myGameObject->GetComponent<AudioComponent>();
		audioComp->StopAll();
	}
}
void Engine::MusicManager::Start()
{
	if (myGameObject->GetComponent<AudioComponent>())
	{
		auto audioComp = myGameObject->GetComponent<AudioComponent>();
		audioComp->AddEvent("Grasslands", "event:/ST/TRACK/Grasslands");
		audioComp->AddEvent("Desert", "event:/ST/TRACK/Desert");
		audioComp->AddEvent("MiniBoss", "event:/ST/TRACK/Miniboss");
		audioComp->AddEvent("FinalBoss", "event:/ST/TRACK/FinalBoss");
		audioComp->AddEvent("MainMenu", "event:/ST/TRACK/Menu");

		audioComp->AddEvent("StonefallScatter", "event:/AMB/Arena/StonefallScatter");
		audioComp->AddEvent("WindScatterArena", "event:/AMB/Arena/WindScatter");
		audioComp->AddEvent("DesertAMB", "event:/AMB/Dessert/DessertAMB");
		audioComp->AddEvent("WindScatterDessert", "event:/AMB/Dessert/WindScatter");
		audioComp->AddEvent("GeyserScatter", "event:/AMB/Grasslands/GeyserScatter");
		audioComp->AddEvent("GrasslandsAMB", "event:/AMB/Grasslands/GrasslandsAMB");
		audioComp->AddEvent("ArenaAMB", "event:/AMB/Arena/ArenaAMB");

		audioComp->ChangeParameter("parameter:/Tension", myParameterValue);
	}

	if (myShouldPlay)
		PlayMusic();
}
void Engine::MusicManager::Execute(eEngineOrder aOrder)
{
	HandleMusicParameters();
	HandleDelayedPlay();
	if (myShouldPlay)
	{
		PlayMusic();
	}
	if (myTriggerFinalBoss)
	{
		myTriggerFinalBoss = false;

		ActivateFinalBoss();
	}
}

void Engine::MusicManager::Reflect(Reflector& aReflector)
{
	//Must be in all components
	Component::Reflect(aReflector);

	aReflector.Reflect(myShouldPlay, "Play On Start");
	aReflector.Reflect(myTriggerFinalBoss, "Trigger final boss music");
	aReflector.Reflect(myParameterValue, "Parameter Value");

	aReflector.ReflectEnum(myLevel, "Level");

}

void Engine::MusicManager::Play()
{
	Stop();
	myShouldPlay = true;
}
void Engine::MusicManager::PlayDelayed(float aTime)
{
	myDelayTimerOn = true;
	myDelayTimer = 0;
	myDelayTime = aTime;
}
void Engine::MusicManager::Stop()
{
	auto audioComp = myGameObject->GetComponent<AudioComponent>();
	audioComp->StopAll();
}

void Engine::MusicManager::SetParameter(const std::string& aParameterName, float aValue)
{
	auto audioComp = myGameObject->GetComponent<AudioComponent>();
	audioComp->ChangeParameter(aParameterName, aValue);
}
void Engine::MusicManager::SetIndividualParameter(int aUUID, const std::string& aEvent, const std::string& aParameterName, float aValue)
{
	auto audioComp = myGameObject->GetComponent<AudioComponent>();
	audioComp->ChangeParameter(aUUID, audioComp->GetEventPath(aEvent), aParameterName, aValue);
}
Engine::eMusicLevel Engine::MusicManager::GetLevel()
{
	return myLevel;
}
void Engine::MusicManager::SetParameter(int aUUID, const std::string& aParameterName, float aValue)
{
	auto audioComp = myGameObject->GetComponent<AudioComponent>();
	switch (myLevel)
	{
	case eMusicLevel::GRASSLANDS:
		audioComp->ChangeParameter(aUUID, audioComp->GetEventPath("Grasslands"), aParameterName, aValue);
		break;

	case eMusicLevel::DESERT:
		audioComp->ChangeParameter(aUUID, audioComp->GetEventPath("Desert"), aParameterName, aValue);
		break;
	}
}

void Engine::MusicManager::SetLevel(eMusicLevel aLevel)
{
	myLevel = aLevel;
}
void Engine::MusicManager::SetNumEnemies(const float& aValue)
{
	myNumEnemies = aValue;
}
void Engine::MusicManager::HandleMusicParameters()
{
	myParameterValue = Math::Remap(myNumEnemies, 0, 25, 0.1f, 1);

	if (myPreviousParameterValue != myParameterValue)
	{
		SetParameter(myGameObject->GetUUID(), "parameter:/Tension", myParameterValue);
	}
	myPreviousParameterValue = myParameterValue;
}
void Engine::MusicManager::HandleDelayedPlay()
{
	if (myDelayTimerOn)
	{
		myDelayTimer += Time::DeltaTime;

		if (myDelayTimer > myDelayTime)
		{
			Play();
			myDelayTimer = 0;
			myDelayTimerOn = false;
		}
	}
}
void Engine::MusicManager::ActivateMiniBoss()
{
	auto audioComp = myGameObject->GetComponent<AudioComponent>();
	audioComp->StopEvent("Grasslands");
	audioComp->StopEvent("Desert");

	audioComp->PlayEvent("MiniBoss");
}
void Engine::MusicManager::ActivateFinalBoss()
{
	auto audioComp = myGameObject->GetComponent<AudioComponent>();
	Stop();

	audioComp->PlayEvent("ArenaAMB");
	audioComp->PlayEvent("FinalBoss");
}
void Engine::MusicManager::PlayMusic()
{
	auto audioComp = myGameObject->GetComponent<AudioComponent>();
	myShouldPlay = false;

	audioComp->StopEvent("MainMenu");
	audioComp->StopEvent("Desert");
	audioComp->StopEvent("Grasslands");
	audioComp->StopEvent("MiniBoss");
	audioComp->StopEvent("FinalBoss");
	audioComp->StopEvent("DesertAMB");
	audioComp->StopEvent("GrasslandsAMB");
	audioComp->StopEvent("MainMenu");

	switch (myLevel)
	{
	case eMusicLevel::GRASSLANDS:
		audioComp->PlayEvent("Grasslands");
		audioComp->PlayEvent("GrasslandsAMB");
		break;
	case eMusicLevel::DESERT:
		audioComp->PlayEvent("Desert");
		audioComp->PlayEvent("DesertAMB");
		break;
	case eMusicLevel::MINIBOSS:
		audioComp->PlayEvent("MiniBoss");
		break;
	case eMusicLevel::FINALBOSS:
		audioComp->PlayEvent("FinalBoss");
		break;
	case eMusicLevel::MAINMENU:
		audioComp->PlayEvent("MainMenu");
		break;
	}
	/*switch (myLevel)
	{
	case eMusicLevel::GRASSLANDS:
		audioComp->PlayEvent("Grasslands");
		break;
	case eMusicLevel::DESERT:
		audioComp->PlayEvent("Desert");
		break;
	case eMusicLevel::MINIBOSS:
		audioComp->PlayEvent("MiniBoss");
		break;
	case eMusicLevel::FINALBOSS:
		audioComp->PlayEvent("FinalBoss");
		break;
	case eMusicLevel::MAINMENU:
		audioComp->PlayEvent("MainMenu");
		break;
	}*/
}