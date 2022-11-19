#pragma once

#include "Engine\GameObject\GameObject.h"
#include "AI/BehaviorTree/Base/BT_Blackboard.h"

class EnemyBase;

class LaserBeam
{
public:
	enum class eFireState
	{
		Ready,
		Charging,
		Firing,
		Cooldown
	};

	LaserBeam();
	~LaserBeam();

	void Init(EnemyBase* anEnemyRef, BT_Blackboard* aBlackboard, float aDamage, float aChargeTime, float aFireTime, float aCooldown);

	void Ignite();

	void Update(float aDeltaTime);

	const eFireState GetFireState() const;
private:
	EnemyBase* myEnemyRef;
	BT_Blackboard* myBlackboard;

	eFireState myFireState = eFireState::Ready;

	float myDamage = 10.f;

	float myLaserTimer = 0.f;

	float myChargeTime = 3.f;
	float myFireTime = 1.5f;
	float myCooldownTime = 4.f;

	float myDamageTimer = 0;
};

