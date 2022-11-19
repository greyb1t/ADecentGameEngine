#pragma once
#include "AI/BehaviorTree/Base/BT_Blackboard.h"
#include "AI/BehaviorTree/Base/BT_Leaf.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/Shortcuts.h"

class BTN_Mortar_Shoot : public BT_Leaf
{
public:
	BTN_Mortar_Shoot(BT_Blackboard::Ptr aBlackBoard, EnemyBase* aEnemyBase) :
		BT_Leaf(aBlackBoard),
		myEnemyBase(aEnemyBase)
	{};

	Status update() override
	{
		myShotTime += Time::DeltaTime;

		if (myBlackboard->getBool("IsDead"))
			return  Status::Success;

		if (myEnemyBase->CheckIfCanShoot(2000.f) && myShotTime > myShotCooldown)
		{
			myShotTime = 0.f;
			if (myBlackboard->GetAnimationController() && myBlackboard->GetAnimationController()->HasTrigger("Shoot"))
				Enemy::EnemyTrigger(*myBlackboard->GetAnimationController(), "Shoot");
		}

		return Status::Success;
	}

private:
	EnemyBase* myEnemyBase = nullptr;

	float myShotTime = 1.5f;
	float myShotCooldown = 2.f;
};

