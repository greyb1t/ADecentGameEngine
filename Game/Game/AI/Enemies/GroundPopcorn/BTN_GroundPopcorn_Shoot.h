#pragma once
#include "AI/BehaviorTree/Base/BT_Blackboard.h"
#include "AI/BehaviorTree/Base/BT_Leaf.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/Shortcuts.h"

class BTN_GroundPopcorn_Shoot : public BT_Leaf
{
public:
	BTN_GroundPopcorn_Shoot(BT_Blackboard::Ptr aBlackBoard, EnemyBase* aEnemyBase) :
		BT_Leaf(aBlackBoard),
		myEnemyBase(aEnemyBase)
	{};

	Status update() override
	{
		myShotTime += Time::DeltaTime;
		if (myEnemyBase->CheckIfCanShoot(2000.f) && myShotTime > myShotCooldown)
		{
			myShotTime = 0.f;
			if (myBlackboard->GetAnimationController() && myBlackboard->GetAnimationController()->HasTrigger("Shoot"))
				Enemy::EnemyTrigger(*myBlackboard->GetAnimationController(), "Shoot");

			myEnemyBase->Shoot(myBlackboard);
			GDebugDrawer->DrawSphere3D(
				DebugDrawFlags::AI,
				myBlackboard->GetOwner()->GetTransform().GetPosition(),
				100.f,
				0.1f,
				{ 1.f, 0.f, 0.f, 1.f });
		}

		return Status::Success;
	}

private:
	EnemyBase* myEnemyBase = nullptr;

	float myShotTime = 0.f;
	float myShotCooldown = 2.f;
};

