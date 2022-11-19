#pragma once

#include "Engine\GameObject\GameObject.h"
#include "AI/BehaviorTree/Base/BT_Blackboard.h"

class EnemyBase;

class GroundPound
{
public:
	enum class eFireState
	{
		Ready,
		Charging,
		Pounding,
		Cooldown
	};

	GroundPound();
	~GroundPound();

	void Init(EnemyBase* anEnemy, float aDamage, float aChargeTime, float aCooldown);

	void InitiateAttack();

	void Attack(BT_Blackboard* aBlackBoard);

	void EndAttack();

	void Update(float aDeltaTime);

	const eFireState GetFireState() const;
private:
	CU::Vector3f myAttackPoint = { 0,0,0 };

	EnemyBase* myEnemyRef;

	eFireState myFireState = eFireState::Ready;

	Engine::VFXComponent* myVFX = nullptr;
	GameObject* mySightLaserVFX = nullptr;

	float myDamage = 10.f;

	float myPoundTimer = 0.f;

	float myChargeTime = 3.f;
	float myCooldownTime = 4.f;

	float myBeginningAttackHeight;
	float myDesiredAttackHeight;
};

