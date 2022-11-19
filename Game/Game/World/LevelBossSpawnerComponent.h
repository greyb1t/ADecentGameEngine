#pragma once

#include "Engine/GameObject/Components/Component.h"
#include "Engine/ResourceManagement/ResourceRef.h"
#include "Engine/ResourceManagement/Resources/GameObjectPrefabResource.h"

class LevelBossSpawnerComponent : public Component
{
public:
	COMPONENT(LevelBossSpawnerComponent, "LevelBossSpawnerComponent");

	~LevelBossSpawnerComponent();

	void Awake() override;
	void Start() override;

	void SpawnLevelBoss();

	void Reflect(Engine::Reflector& aReflector) override;

private:
	GameObjectPrefabRef myLevelBossPrefab{};

	static inline std::vector<LevelBossSpawnerComponent*> ourSpawners{};
	static inline bool ourSpawnersInitialized = false;
};