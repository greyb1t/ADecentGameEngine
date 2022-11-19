#pragma once

#include "Projectile.h"
#include "Engine\GameObject\GameObject.h"

class EnemyBase;

class PoisonCloud : public Projectile
{
public:
	COMPONENT(PoisonCloud, "PoisonCloud");
	PoisonCloud() = default;
	PoisonCloud(GameObject* aGameObject);
	~PoisonCloud();

	void Init(EnemyBase* anEnemy, float aDamage, float aDuration, float aRadius);
	void Execute(Engine::eEngineOrder aOrder) override;

	void Update(float aDeltaTime);
private:
	EnemyBase* myEnemyRef = nullptr;

	float myCurrentRadius = 0;
	float myRadius = 0;
	float myDamageHeight = 50 + 100; // Base value 50 for gas and 100 for half player
	float myDamage = 10.f;
	float myDuration = 4.f;
	float myDurationTimer = 0;

	float myDamageTimer = 0;
};

