#pragma once
#include "EnemyScalableStats.h"
#include "AI/BehaviorTree/Base/BT_Blackboard.h"
#include "Engine\GameObject\GameObject.h"
#include "Engine/GameObject/Components/Component.h"
#include "Engine/GameObject/Components/DestructComponent.h"

enum class eEnemyShootType
{
	RapidPopcorn,
	GroundPopcorn,
	BurstTank,
	Laser,
	Mortar,

	Count
};

enum class EnemyDeathType
{
	Explode,
	Delay,

	Count
};

class EnemyBase : public Component
{
public:
	EnemyBase(GameObject* aGameObject);
	EnemyBase() = default;
	~EnemyBase();

	void Start() override;

	void Execute(Engine::eEngineOrder aOrder) override;

	void Update();

	void Reflect(Engine::Reflector& aReflector) override;

	void Render() override;

	virtual void OnDamage();

	void SetDeSpawnFunction(std::function<void()> aDeSpawnFunction);

	void SetTargetRotationRadians(float aRadians, float aRotationSmoothness);
	void UpdateSmoothRotation();

	void RotateTowardsPlayer(float aRotationSmoothness);
	bool CheckIfCanShoot(float aShootRange);
	void Shoot(BT_Blackboard::Ptr aBlackBoard) const;
	void BurstShoot(Enemy::eBurstShootData aBurstShootData, BT_Blackboard::Ptr aBlackBoard) const;
	void Mortar(Enemy::eMortarShootData aMortarShootData) const;

	void SetEnemyShootType(eEnemyShootType aEnemyShootType);

	void CheckIfToFarFromPlayer();

	//Debuffs mm
	void SetUpStun(std::function<void(float)> aStunFunction);
	bool SetStunValues(float aStunTime);
	void SetUpSlow(std::function<void(float, float)> aSlowFunction);
	bool SetSlowValues(float aSlowPercentage, float aSlowTime);
	void SetUpKnockBack(std::function<void(Vec3f)> aSlowFunction);
	bool SetKnockBackValues(Vec3f aDirection);

	void UpdateDebuffTimers();

	void Knockback(Vec3f aDirection);

	EnemyDeathType GetEnemyDeathType();
	void SetEnemyDeathType(EnemyDeathType aEnemyDeathType);
	virtual void SpawnDeathSound();
protected:
	EnemyScalableValues* myEnemyScalableStats = nullptr;

	//Debuffstats
	bool myStunned = false;
	float myStunTimer = 0;

	bool mySlowed = false;

	bool myKnockedBack = false;
	Vec3f myKnockBackDirection = { 0,0,0 };
private:
	eEnemyShootType myEnemyShootType = eEnemyShootType::Count;
	EnemyDeathType myEnemyDeathType = EnemyDeathType::Delay;
	float myCurrentRotationRadians = 0.f;
	float myTargetRotationRadians = 0.f;
	float myRotationSmoothness = 16.f;

	std::function<void()> myDeSpawnFunction;

	std::function<void(float)> myStunFunction;
	std::function<void(float, float)> mySlowFunction;
	std::function<void(Vec3f)> myKnockBackFunction;
	bool myIsKnocked = false;

	//Knockback variables
	float myFlyProgress; 
	float myFlyDuration = 1;
};