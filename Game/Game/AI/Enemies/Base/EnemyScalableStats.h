#pragma once
#include "EnemyUtility.h"
#include "Engine/GameObject/Components/Component.h"

struct EnemyScalableValues
{
	float myHealth = 100;
	float myDamage = 5.f;
};
class EnemyScalableStats : public Component
{
public:
	COMPONENT(EnemyScalableStats, "EnemyScalableStats");

	EnemyScalableStats() = default;
	EnemyScalableStats(GameObject* aGameObject);
	~EnemyScalableStats() = default;

	void Start() override;

	void Reflect(Engine::Reflector& aReflector) override;

	void Execute(Engine::eEngineOrder aOrder) override;
	void Render() override;

	EnemyScalableValues& GetScalableValues();
	void SetEnemyScalableValues(EnemyScalableValues aEnemyScalableValues);

	void SetStatMultiplier(float aStatMultiplier);
private:
	EnemyScalableValues myEnemyScalableValues;
	float myStatMultiplier = 0.f;
};

