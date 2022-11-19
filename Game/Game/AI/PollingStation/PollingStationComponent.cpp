#include "pch.h"
#include "PollingStationComponent.h"

#include "Engine/Shortcuts.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/Scene/FolderScene.h"
#include "Engine/Reflection/Reflector.h"
#include "Engine/GameObject/Components/PointLightComponent.h"
#include <Engine/GameObject/Components/MusicManager.h>
#include <Engine/AudioManager.cpp>

PollingStationComponent::PollingStationComponent(GameObject* aGameObject) :
	Component(aGameObject)
{
}

void PollingStationComponent::Start()
{
	Component::Start();

	CreatePointLightPool();
}

void PollingStationComponent::Reflect(Engine::Reflector& aReflector)
{
	Component::Reflect(aReflector);
	aReflector.Reflect(myMaxNumberOfEnemies, "MAX Number OF Enemies At a Time");

}

void PollingStationComponent::Execute(Engine::eEngineOrder aOrder)
{
	Component::Execute(aOrder);
	IncrementDelayedPositions();
	DebugDraw();
	//LOG_INFO(LogType::Jesper) << "Number of enemies in level: " << myNumberOfEnemies;
}

void PollingStationComponent::Render()
{
	Component::Render();
}

void PollingStationComponent::Setplayer(Component* aComponent)
{
	myPlayerComponent = aComponent;
	AudioManager::GetInstance()->SetPlayer(myPlayerComponent->GetTransform().GetGameObjectShared());
}

const Vec3f PollingStationComponent::GetPlayerPos()
{
	return myPlayerComponent->GetGameObject()->GetTransform().GetPosition();
}

const Vec3f PollingStationComponent::GetDelayedPlayerPos(int aDelayValue)
{
	int correctedValue = aDelayValue;

	if (correctedValue > 999)
	{
		correctedValue = 999;
	}
	else if (correctedValue < 0)
	{
		correctedValue = 0;
	}

	//correctedValue = 999 - correctedValue;
	if (correctedValue >= myDelayedPlayerPositions.size())
	{
		correctedValue = myDelayedPlayerPositions.size() - 1;
	}
	Vec3f temp = GetPlayerPos();
	return myDelayedPlayerPositions[correctedValue];
}

GameObject* PollingStationComponent::GetPlayer()
{
	if (!myPlayerComponent)
	{
		LOG_ERROR(LogType::Game) << "Could not find player component. Returning nullptr.";
		return nullptr;
	}
	return myPlayerComponent->GetGameObject();
}

Component* PollingStationComponent::GetPlayerComponent()
{
	return myPlayerComponent;
}


void PollingStationComponent::DebugDraw()
{

}

void PollingStationComponent::EnemyCounterIncrease()
{
	myNumberOfEnemies++;
	UpdateMusicEnemyCount();
}

void PollingStationComponent::EnemyCounterDecrease()
{
	myNumberOfEnemies--;
	UpdateMusicEnemyCount();
}
void PollingStationComponent::UpdateMusicEnemyCount()
{
	if (myGameObject->GetSingletonComponent<Engine::MusicManager>())
	{
		auto* musicManager = myGameObject->GetSingletonComponent<Engine::MusicManager>();

		musicManager->SetNumEnemies(myNumberOfEnemies);
	}
}
int PollingStationComponent::GetNumberOfEnemiesInLevel()
{
	return myNumberOfEnemies;
}

int PollingStationComponent::GetMAXNumberOfEnemiesInLevel()
{
	return myMaxNumberOfEnemies;
}

void PollingStationComponent::CreatePointLightPool()
{
	for (int i = 0; i < ourMaxPointLightsInPool; ++i)
	{
		GameObject* g = myGameObject->GetScene()->AddGameObject<GameObject>();
		g->SetActive(false);
		g->AddComponent<Engine::PointLightComponent>(false);

		myPoolQueue.push(g);
	}
}

GameObject* PollingStationComponent::GetPointlightFromPool()
{
	if (myPoolQueue.empty())
	{
		return nullptr;
	}

	GameObject* g = myPoolQueue.front();
	myPoolQueue.pop();
	return g;
}

void PollingStationComponent::ReturnPointLightToPool(GameObject* aPointLightGameObject)
{
	if (!aPointLightGameObject)
	{
		return;
	}

	aPointLightGameObject->SetActive(false);
	myPoolQueue.push(aPointLightGameObject);
}

void PollingStationComponent::IncrementDelayedPositions()
{
	if (myDelayedPlayerPositions.size() < 1001)
	{
		myDelayedPlayerPositions.push_back(GetPlayerPos());
	}
	else
	{
		myDelayedPlayerPositions.erase(myDelayedPlayerPositions.begin());
		myDelayedPlayerPositions.push_back(GetPlayerPos());
	}
}



