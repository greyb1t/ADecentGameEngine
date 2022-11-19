#include "pch.h"
#include "LevelBossSpawnerComponent.h"

#include "Common/Random.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"

LevelBossSpawnerComponent::~LevelBossSpawnerComponent()
{
	ourSpawners.erase(std::remove(ourSpawners.begin(), ourSpawners.end(), this), ourSpawners.end());

	if(ourSpawners.empty())
	{
		ourSpawnersInitialized = false;
	}
}

void LevelBossSpawnerComponent::Awake()
{
	ourSpawners.push_back(this);	
}

void LevelBossSpawnerComponent::Start()
{
	if(ourSpawnersInitialized)
	{
		return;
	}

	size_t index = Random::RandomUInt(0, ourSpawners.size() - 1);
	ourSpawners[index]->SpawnLevelBoss();
	
	ourSpawnersInitialized = true;
}

void LevelBossSpawnerComponent::SpawnLevelBoss()
{
	GameObject& boss = myLevelBossPrefab->Get().Instantiate(*GetGameObject()->GetScene());
	Transform& transform = boss.GetTransform();

	transform.SetPosition(GetTransform().GetPosition());
	transform.SetRotation(GetTransform().GetRotation());
}

void LevelBossSpawnerComponent::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myLevelBossPrefab, "Level Boss Prefab");
}