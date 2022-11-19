#include "pch.h"
#include "AIDirector.h"

#include "AI/Enemies/Base/EnemyBase.h"
#include "AI/Enemies/Base/EnemyScalableStats.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "Engine/Shortcuts.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"
#include "Engine/Scene/FolderScene.h"
#include "Engine/Reflection/Reflector.h"
#include "Engine/ResourceManagement/Resources/GameObjectPrefabResource.h"
#include "GameManager/GameManager.h"
#include "AI/FinalBoss/FinalBoss.h"
#include "AI/FinalBoss/FinalBossPhase.h"

void EnemyPrefabData::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myEnemyPrefab, "Enemies To Spawn");
	aReflector.Reflect(myEnemyType, "Type");
	aReflector.Reflect(mySpawnCostValue, "Spawn Cost Value");
}

void WaveEmitter::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Separator();
	aReflector.Reflect(myIsActive, "IsActive");
	aReflector.Reflect(myName, "Name");
	aReflector.Reflect(myTotalSpawnValue, "Total Spawn Value");
	aReflector.Reflect(myTimeBetweenWaves, "Time Between Waves");
	//aReflector.Reflect(myEnemyPrefabData, "Enemy Prefab Data");
	aReflector.Separator();
}

void Tier::Reflect(Engine::Reflector& aReflector)
{

	aReflector.Separator();
	aReflector.Reflect(myStatMultiplier, "Stat Multiplier");
	aReflector.Reflect(myEnemyPrefabData, "Enemy Prefab Data");
	aReflector.Separator();
	aReflector.Reflect(myWaveEmitters, "Wave Emitters");
	aReflector.Separator();

}

AIDirector::AIDirector(GameObject* aGameObject) :
	Component(aGameObject)
{
}

void AIDirector::Start()
{
	Component::Start();

	if (!myFinalBossGameObject.Get())
	{
		LOG_ERROR(LogType::Game) << "Missing final boss in AIDirector";
	}
}

void AIDirector::SetPlayer(GameObject* aPlayerGameObject)
{
	myPlayerGameObject = aPlayerGameObject;
}

void AIDirector::Reflect(Engine::Reflector& aReflector)
{
	Component::Reflect(aReflector);

	aReflector.Separator();
	aReflector.Reflect(myOuterEnemySpawnRadius, "outer Enemy Spawn Radius");
	aReflector.Reflect(myInnerEnemySpawnRadius, "Inner Enemy Spawn Radius");
	aReflector.Reflect(myFlyingMinSpawnHeight, "Flying Min Spawn Height");
	aReflector.Reflect(myFlyingMaxSpawnHeight, "Flying Max Spawn Height");

	aReflector.Separator();
	aReflector.Reflect(myEndlessMultiplier, "Endless Multiplier");

	aReflector.Separator();
	aReflector.Reflect(myEnemySpawnPortalPrefab, "Enemy Spawn Portal Prefab");

	aReflector.Separator();
	aReflector.Reflect(myEnemySpawnPortalPrefabFly, "Enemy Spawn Portal Prefab Fly");

	aReflector.Separator();
	aReflector.Reflect(myLaserPrefab, "Laser Prefab");
	aReflector.Reflect(myLaserSightPrefab, "LaserSight Prefab");

	aReflector.Separator();
	aReflector.Reflect(myEnemyDespawnDistacne, "Enemy Despawn Distance");
	aReflector.Reflect(myPopCornProjectile, "PopCorn Projectile");
	aReflector.Reflect(myAirTankProjectile, "AirTank Projectile");
	aReflector.Reflect(myMortarIndicator, "Mortar Indicator");

	aReflector.Separator();

	aReflector.Reflect(myTestingDifficulty, "Testing Difficulty");

#ifdef RETAIL
	myTestingDifficulty = false;
#endif

	if (myTestingDifficulty == true)
	{
		if (const auto gm = GameManager::GetInstance())
		{
			GameDifficulty difficulty = gm->GetGameDifficulty();
			if (aReflector.ReflectEnum(difficulty, "Difficulty") & Engine::ReflectorResult_Changed)
			{
				gm->SetGameDifficulty(difficulty);
			}
		}
	}

	aReflector.Header("Tiers");
	for (int i = 0; i < myTierList.size(); i++)
	{
		const std::string name = GameDifficulty::_from_integral(i)._to_string();
		aReflector.Reflect(myTierList[i], std::string(name));
	}


	aReflector.Separator();
	//aReflector.Header("Wave Emitters");
	//aReflector.Reflect(myWaveEmitters, "Wave Emitters");
	//aReflector.Separator();


	//aReflector.Header("TestHeader");

	aReflector.Reflect(myPhase1Tier, "Phase 1 Tier");
	aReflector.Reflect(myPhase2Tier, "Phase 2 Tier");
	aReflector.Reflect(myFinalBossGameObject, "Final Boss GameObject");
}

void AIDirector::Execute(Engine::eEngineOrder aOrder)
{
	Component::Execute(aOrder);
	Update();
}

void AIDirector::Update()
{
	if (myIsAtFinalBoss)
	{
		UpdateFinalBossEnemySpawnPatterns();
	}
	else
	{
		UpdateEnemySpawnPatterns();
	}

	if (myStartEndlessTimer)
	{
		myEndlessTimer += Time::DeltaTime;
		if (myEndlessTimer > myEndlessTime)
		{
			GameManager::GetInstance()->IncreaseEndlessCounter();
			myEndlessTimer = 0.f;
		}
	}

	DebugDraw();
}

float AIDirector::GetEnemyDespawnDistacne()
{
	return myEnemyDespawnDistacne;
}

void AIDirector::UpdateEnemySpawnPatterns()
{
#ifndef _RETAIL
	if (GetEngine().GetInputManager().IsKeyDown(Common::KeyCode::P))
		myIsDisable = !myIsDisable;
#endif


	if (myIsDisable)
		return;
	const int index = GameManager::GetInstance()->GetGameDifficulty();

	/*for (size_t i = 0; i < myWaveEmitters.size(); i++)
	{
		if (!myWaveEmitters[i].myIsActive)
			continue;

		myWaveEmitters[i].myWaveSpawnTimer += Time::DeltaTime;
		if (myWaveEmitters[i].myWaveSpawnTimer > myWaveEmitters[i].myTimeBetweenWaves)
		{
			myWaveEmitters[i].myWaveSpawnTimer = 0.f;
			SpawnWave(myWaveEmitters[i], myTierList[index]);
		}
	}*/
	for (size_t i = 0; i < myTierList[index].myWaveEmitters.size(); i++)
	{
		if (!myTierList[index].myWaveEmitters[i].myIsActive)
			continue;

		myTierList[index].myWaveEmitters[i].myWaveSpawnTimer += Time::DeltaTime;
		if (myTierList[index].myWaveEmitters[i].myWaveSpawnTimer > myTierList[index].myWaveEmitters[i].myTimeBetweenWaves)
		{
			myTierList[index].myWaveEmitters[i].myWaveSpawnTimer = 0.f;
			SpawnWave(myTierList[index].myWaveEmitters[i], myTierList[index]);
		}
	}
}

void AIDirector::UpdateFinalBossEnemySpawnPatterns()
{
#ifndef _RETAIL
	if (GetEngine().GetInputManager().IsKeyDown(Common::KeyCode::P))
		myIsDisable = !myIsDisable;
#endif

	if (myIsDisable)
		return;
	// const int index = GameManager::GetInstance()->GetGameDifficulty();

	if (!myFinalBossGameObject.Get())
	{
		return;
	}

	const FB::FinalBoss& finalBoss = *myFinalBossGameObject.Get()->GetComponent<FB::FinalBoss>();

	Tier* choosenTier = nullptr;

	switch (finalBoss.GetPhase())
	{
	case FB::FinalBossPhase::Phase1:
		choosenTier = &myPhase1Tier;
		break;
	case FB::FinalBossPhase::Phase2:
		choosenTier = &myPhase2Tier;
		break;
	default:
		break;
	}

	if (!choosenTier)
	{
		return;
	}

	for (size_t i = 0; i < choosenTier->myWaveEmitters.size(); i++)
	{
		if (!choosenTier->myWaveEmitters[i].myIsActive)
			continue;

		choosenTier->myWaveEmitters[i].myWaveSpawnTimer += Time::DeltaTime;
		if (choosenTier->myWaveEmitters[i].myWaveSpawnTimer > choosenTier->myWaveEmitters[i].myTimeBetweenWaves)
		{
			choosenTier->myWaveEmitters[i].myWaveSpawnTimer = 0.f;

			SpawnWave(choosenTier->myWaveEmitters[i], *choosenTier);
		}
	}
}

void AIDirector::SpawnWave(const WaveEmitter& aEnemySpawnPattern, Tier& aTier)
{
	const auto pollingStationComp = myGameObject->GetSingletonComponent<PollingStationComponent>();
	if (pollingStationComp->GetNumberOfEnemiesInLevel() >= pollingStationComp->GetMAXNumberOfEnemiesInLevel())
		return;

	int spawnValueLeft = aEnemySpawnPattern.myTotalSpawnValue;
	const int numOfDifferentEnemies = static_cast<int>(aTier.myEnemyPrefabData.size());

	int cheapestCostValue = std::numeric_limits<int>::max();
	for (const auto& i : aTier.myEnemyPrefabData)
	{
		if (i.mySpawnCostValue < cheapestCostValue)
			cheapestCostValue = i.mySpawnCostValue;
	}

	int safetyCounter = 0;
	while (spawnValueLeft >= cheapestCostValue && safetyCounter < 1000)
	{
		safetyCounter++;
		const int suggestedSpawnIndex = Random::RandomInt(0, numOfDifferentEnemies - 1);

		spawnValueLeft -= aTier.myEnemyPrefabData[suggestedSpawnIndex].mySpawnCostValue;
		if (spawnValueLeft < 0)
		{
			spawnValueLeft += aTier.myEnemyPrefabData[suggestedSpawnIndex].mySpawnCostValue;
			continue;
		}

		Vec3f spawnPos = GetPositionInCircle();
		Vec3f rayPos = spawnPos;
		rayPos.y += myRayHeightOffset;

		constexpr float rayRange = 30000.f;

		Vec3f rayDirection = { 0.f, -1.f, 0.f };
		rayDirection.Normalize();
		RayCastHit hit;
		if (myGameObject->GetScene()->RayCast(
			rayPos,
			rayDirection,
			rayRange,
			eLayer::NAVMESH,
			hit))
		{
			spawnPos = hit.Position;
			myNavmeshHitPos = hit.Position;

			if (myGameObject->GetScene()->RayCast(
				spawnPos + Vec3f(0,200,0),
				rayDirection,
				200,
				eLayer::DEFAULT | eLayer::GROUND,
				hit))
			{
				spawnPos = hit.Position; 
			}

			GDebugDrawer->DrawSphere3D(
				DebugDrawFlags::AI,
				hit.Position,
				10.f,
				30.f,
				{ 1.f, 0.f, 1.f, 1.f });

			GDebugDrawer->DrawSphere3D(
				DebugDrawFlags::AI,
				rayPos,
				5.f,
				5.f,
				{ 0.f, 1.f, 0.f, 1.f });

			GDebugDrawer->DrawLine3D(
				DebugDrawFlags::AI,
				rayPos,
				hit.Position,
				5.f,
				{ 0.f, 1.f, 0.f, 1.f });
		}
		else
		{
			GDebugDrawer->DrawSphere3D(
				DebugDrawFlags::AI,
				rayPos,
				5.f,
				5.f,
				{ 1.f, 0.f, 0.f, 1.f });

			GDebugDrawer->DrawLine3D(
				DebugDrawFlags::AI,
				rayPos,
				rayPos + rayDirection * rayRange,
				5.f,
				{ 1.f, 0.f, 0.f, 1.f });
			continue;
		}

		Engine::GameObjectPrefab& enemyPrefab = aTier.myEnemyPrefabData[suggestedSpawnIndex].myEnemyPrefab->Get();
		auto& enemy = enemyPrefab.Instantiate(*myGameObject->GetScene());

		switch (static_cast<Enemy::eEnemyType>(aTier.myEnemyPrefabData[suggestedSpawnIndex].myEnemyType.GetValue()))
		{
		case Enemy::eEnemyType::Ground:
			spawnPos.y += 40.f;
			break;
		case Enemy::eEnemyType::Flying:
			spawnPos.y += Random::RandomFloat(myFlyingMinSpawnHeight, myFlyingMaxSpawnHeight);
			break;
		}

		const auto enemyScalableStatsComp = enemy.GetComponent<EnemyScalableStats>();
		auto& enemyScalableStats = enemyScalableStatsComp->GetScalableValues();

		const float difficultyMultiplier = GameManager::GetInstance()->GetGameSettings().myDifficultyMultiplier;

		if (GameManager::GetInstance()->GetGameDifficulty() == +GameDifficulty::Endless)
		{
			myStartEndlessTimer = true;
			enemyScalableStats.myHealth = (enemyScalableStats.myHealth * (aTier.myStatMultiplier +
				(myEndlessMultiplier * static_cast<float>(GameManager::GetInstance()->GetEndlessCounter())))) * difficultyMultiplier;

			enemyScalableStats.myDamage = (enemyScalableStats.myDamage * (aTier.myStatMultiplier +
				(myEndlessMultiplier * static_cast<float>(GameManager::GetInstance()->GetEndlessCounter())))) * difficultyMultiplier;
		}
		else
		{
			enemyScalableStats.myHealth *= (aTier.myStatMultiplier + 1.f) * difficultyMultiplier;
			enemyScalableStats.myDamage *= (aTier.myStatMultiplier + 1.f) * difficultyMultiplier;
		}

		enemy.GetTransform().SetPosition(spawnPos);
		enemy.GetSingletonComponent<PollingStationComponent>()->EnemyCounterIncrease();

		switch (static_cast<Enemy::eEnemyType>(aTier.myEnemyPrefabData[suggestedSpawnIndex].myEnemyType.GetValue()))
		{
		case Enemy::eEnemyType::Ground:
		{
			Engine::GameObjectPrefab& enemySpawnPortalPrefab = myEnemySpawnPortalPrefab->Get();
			auto& enemySpawnPortal = enemySpawnPortalPrefab.Instantiate(*myGameObject->GetScene());
			spawnPos.y += 20.f;
			enemySpawnPortal.GetTransform().SetPosition(spawnPos);
		}
		break;
		case Enemy::eEnemyType::Flying:
		{
			Engine::GameObjectPrefab& enemySpawnPortalPrefab = myEnemySpawnPortalPrefabFly->Get();
			auto& enemySpawnPortal = enemySpawnPortalPrefab.Instantiate(*myGameObject->GetScene());
			spawnPos.y += 20.f;
			enemySpawnPortal.GetTransform().SetPosition(spawnPos);
		}
		break;
		}
	}

}

void AIDirector::Render()
{
	Component::Render();
}

void AIDirector::DisableAllWaveEmitters()
{
	//const int index = GameManager::GetInstance()->GetGameDifficulty();

	for (int i = 0; i < myTierList.size(); i++)
	{
		for (auto& myWaveEmitter : myTierList[i].myWaveEmitters)
		{
			myWaveEmitter.myIsActive = false;
		}
	}

	/*for (auto& myWaveEmitter : myTierList[index].myWaveEmitters)
	{
		myWaveEmitter.myIsActive = false;
	}*/
	LOG_INFO(LogType::Game) << "Disable Wave Emitter";
}

void AIDirector::EnableWaveEmitter(const std::string& aWaveEmitterName)
{
	const int index = GameManager::GetInstance()->GetGameDifficulty();
	for (auto& myWaveEmitter : myTierList[index].myWaveEmitters)
	{
		if (myWaveEmitter.myName == aWaveEmitterName)
		{
			myWaveEmitter.myIsActive = true;
		}
	}
	LOG_INFO(LogType::Game) << aWaveEmitterName << " wave emitter activated";
}

const Vec3f AIDirector::GetPlayerPosition() const
{
	return myGameObject->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos();
}

GameObjectPrefabRef& AIDirector::GetPopCornProjectile()
{
	return myPopCornProjectile;
}

GameObjectPrefabRef& AIDirector::GetAirTankProjectile()
{
	return myAirTankProjectile;
}

GameObjectPrefabRef& AIDirector::GetMortarIndicator()
{
	return myMortarIndicator;
}

GameObjectPrefabRef& AIDirector::GetLaserPrefab()
{
	return myLaserPrefab;
}

GameObjectPrefabRef& AIDirector::GetLaserSightPrefab()
{
	return myLaserSightPrefab;
}

void AIDirector::RespawnEnemy(GameObject& aGO)
{
	const auto enemyBase = aGO.GetComponent<EnemyBase>();

	auto spawnPos = GetPositionInCircle();

	const auto savedYpos = aGO.GetTransform().GetPosition().y;
	spawnPos.y = savedYpos;

	switch (enemyBase->GetEnemyDeathType())
	{
	case EnemyDeathType::Delay:
	{
		Engine::GameObjectPrefab& enemySpawnPortalPrefab = myEnemySpawnPortalPrefab->Get();
		auto& enemySpawnPortal = enemySpawnPortalPrefab.Instantiate(*myGameObject->GetScene());
		spawnPos.y += 20.f;
		enemySpawnPortal.GetTransform().SetPosition(spawnPos);
	}
	break;
	case EnemyDeathType::Explode:
	{
		Engine::GameObjectPrefab& enemySpawnPortalPrefab = myEnemySpawnPortalPrefabFly->Get();
		auto& enemySpawnPortal = enemySpawnPortalPrefab.Instantiate(*myGameObject->GetScene());
		spawnPos.y += 20.f;
		enemySpawnPortal.GetTransform().SetPosition(spawnPos);
	}
	break;
	default:
		break;
	}

	aGO.GetTransform().ResetMovement();
	aGO.GetTransform().SetPosition(spawnPos);
	GDebugDrawer->DrawSphere3D(
		DebugDrawFlags::AI,
		spawnPos,
		5.f,
		5.f,
		{ 1.f, 0.f, 1.f, 1.f });
}

void AIDirector::SetShouldDespawnAll(const bool aValue)
{
	myShouldDespawnAll = aValue;
}

bool AIDirector::ShouldDespawnAll() const
{
	return myShouldDespawnAll;
}

void AIDirector::SetIsAtFinalBoss(const bool aIsAtFinalBoss)
{
	myIsAtFinalBoss = aIsAtFinalBoss;
}

Vec3f AIDirector::GetPositionInCircle()
{
	bool validPos = false;
	int safetyCounter = 0;

	Vec3f spawnPos = { 0.f, GetPlayerPosition().y + 100.f, 0.f };
	const Vec3f playerPos = GetPlayerPosition();

	while (validPos == false)
	{
		safetyCounter++;

		float r = myOuterEnemySpawnRadius * sqrt(Random::RandomFloat(0.f, 1.f));
		float theta = Random::RandomFloat(0.f, 1.f) * 2 * Math::PI;

		spawnPos.x = playerPos.x + r * cos(theta);
		spawnPos.z = playerPos.z + r * sin(theta);

		const float distance = (playerPos - spawnPos).Length();
		if (distance > myInnerEnemySpawnRadius)
		{
			return spawnPos;
		}
		if (safetyCounter > 100)
		{
			validPos = true;
		}
	}
	LOG_WARNING(LogType::Jesper) << "AIDirector iterated to many times to try to spawn enemy";
	return { 0.f, 0.f, 0.f };
}


void AIDirector::DebugDraw()
{
	DebugDrawSpawnCircle();

	GDebugDrawer->DrawSphere3D(
		DebugDrawFlags::AI,
		GetPlayerPosition(),
		10.f,
		0.f,
		{ 1.f, 1.f, 0.f, 1.f });

	if (myGameObject->GetSingletonComponent<PollingStationComponent>()->GetPlayer() != nullptr)
	{
		GDebugDrawer->DrawCircle3D(
			DebugDrawFlags::AI,
			myGameObject->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos(),
			myEnemyDespawnDistacne,
			0.f,
			{ 1.f, 0.f, 1.f, 1.f });
	}
}

void AIDirector::DebugDrawSpawnCircle()
{
	const auto playerPos = GetPlayerPosition();
	GDebugDrawer->DrawCircle3D(
		DebugDrawFlags::AI,
		playerPos,
		myOuterEnemySpawnRadius,
		0.f,
		{ 0.f, 0.f, 1.f, 1.f });
	GDebugDrawer->DrawCircle3D(
		DebugDrawFlags::AI,
		playerPos,
		myInnerEnemySpawnRadius,
		0.f,
		{ 0.f, 0.f, 1.f, 1.f });
}
