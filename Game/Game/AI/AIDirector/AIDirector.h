#pragma once
#include "AI/Enemies/Base/EnemyUtility.h"
#include "Engine/GameObject/Components/Component.h"
#include "Engine\ResourceManagement\ResourceRef.h"

struct EnemyPrefabData : Engine::Reflectable
{
	void Reflect(Engine::Reflector& aReflector) override;

	GameObjectPrefabRef myEnemyPrefab;
	Enemy::EnemyType myEnemyType;
	int mySpawnCostValue = 0;
};

struct WaveEmitter : Engine::Reflectable
{
	void Reflect(Engine::Reflector& aReflector) override;

	bool myIsActive = false;

	std::string myName = "Undefined";
	int myTotalSpawnValue = 0;
	float myTimeBetweenWaves = 0.f;
	float myWaveSpawnTimer = 0.f;
};

struct Tier : Engine::Reflectable
{
	void Reflect(Engine::Reflector& aReflector) override;

	float myStatMultiplier = 0.0f;
	std::vector<EnemyPrefabData> myEnemyPrefabData;
	std::vector<WaveEmitter> myWaveEmitters;

};

struct FinalBossDirector : public Engine::Reflectable
{
	struct Phase : public Engine::Reflectable
	{
		void Reflect(Engine::Reflector& aReflector) override
		{

		}

		int myMaxEnemies = 10;
	};

	void Reflect(Engine::Reflector& aReflector) override
	{
		aReflector.Reflect(myPhase1, "Phase 1");
		aReflector.Reflect(myPhase2, "Phase 2");
	}

	Phase myPhase1;
	Phase myPhase2;
};

class AIDirector : public Component
{
public:
	COMPONENT_SINGLETON(AIDirector, "AIDirector");

	AIDirector() = default;
	AIDirector(GameObject* aGameObject);
	~AIDirector() = default;

	void Start() override;

	void SetPlayer(GameObject* aPlayerGameObject);

	void Reflect(Engine::Reflector& aReflector) override;

	void Execute(Engine::eEngineOrder aOrder) override;
	void Update();
	float GetEnemyDespawnDistacne();
	void Render() override;

	void DisableAllWaveEmitters();
	void EnableWaveEmitter(const std::string& aWaveEmitterName);
	GameObjectPrefabRef& GetPopCornProjectile();
	GameObjectPrefabRef& GetAirTankProjectile();
	GameObjectPrefabRef& GetMortarIndicator();
	GameObjectPrefabRef& GetLaserPrefab();
	GameObjectPrefabRef& GetLaserSightPrefab();

	void RespawnEnemy(GameObject& aGO);

	void SetShouldDespawnAll(const bool aValue);
	bool ShouldDespawnAll() const; 

	void SetIsAtFinalBoss(const bool aIsAtFinalBoss);

private:
	const Vec3f GetPlayerPosition() const;
	void UpdateEnemySpawnPatterns();
	void UpdateFinalBossEnemySpawnPatterns();
	void DebugDraw();
	void DebugDrawSpawnCircle();
	Vec3f GetPositionInCircle();
	void SpawnWave(const WaveEmitter& aEnemySpawnPattern, Tier& aTier);

	//void RayCastOnGround();

	GameObject* myPlayerGameObject = nullptr;
	std::array<Tier, static_cast<int>(GameDifficulty::_size())> myTierList;

	//std::vector<WaveEmitter> myWaveEmitters;
	GameObjectPrefabRef myEnemySpawnPortalPrefab;
	GameObjectPrefabRef myEnemySpawnPortalPrefabFly;


	Vec3f myNavmeshHitPos = { 0.f, 0.f, 0.f };
	float myRayHeightOffset = 7000.f;
	float myOuterEnemySpawnRadius = 500.f;
	float myInnerEnemySpawnRadius = 200.f;
	float myFlyingMinSpawnHeight = 300.f;
	float myFlyingMaxSpawnHeight = 700.f;

	float myEndlessMultiplier = 0.1f;
	float myEndlessTimer = 0.f;
	float myEndlessTime = 60.f;
	bool myStartEndlessTimer = false;

	bool myIsDisable = false;

	float myEnemyDespawnDistacne = 4000.f;
	GameObjectPrefabRef myPopCornProjectile;
	GameObjectPrefabRef myAirTankProjectile;
	GameObjectPrefabRef myMortarIndicator;

	GameObjectPrefabRef myLaserPrefab;
	GameObjectPrefabRef myLaserSightPrefab;

	bool myTestingDifficulty = false;

	bool myShouldDespawnAll = false;

	bool myIsAtFinalBoss = false;

	Tier myPhase1Tier;
	Tier myPhase2Tier;

	Engine::GameObjectRef myFinalBossGameObject;
};
