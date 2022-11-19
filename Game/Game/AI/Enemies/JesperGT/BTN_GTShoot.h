#pragma once
#include "AI/BehaviorTree/Base/BT_Blackboard.h"
#include "AI/BehaviorTree/Base/BT_Leaf.h"
#include "AI/Enemies/Base/EnemyBase.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/Shortcuts.h"

class BTN_GTShoot : public BT_Leaf
{
public:
	BTN_GTShoot(BT_Blackboard::Ptr aBlackBoard, EnemyBase* aEnemyBase) :
		BT_Leaf(aBlackBoard),
		myEnemyBase(aEnemyBase)
	{};

	Status update() override
	{
		myShotTime += Time::DeltaTime;
		myEnemyBase->RotateTowardsPlayer(16.f);

		if (myEnemyBase->CheckIfCanShoot(5000.f) && myShotTime > myShotCooldown)
		{
			myShotTime = 0.f;
			if (myBlackboard->GetAnimationController() && myBlackboard->GetAnimationController()->HasTrigger("Shoot"))
				Enemy::EnemyTrigger(*myBlackboard->GetAnimationController(), "Shoot");

			myEnemyBase->RotateTowardsPlayer(16.f);

			myEnemyBase->Shoot(myBlackboard);
			/*GDebugDrawer->DrawSphere3D(
				DebugDrawFlags::AI,
				myBlackboard->GetOwner()->GetTransform().GetPosition(),
				100.f,
				0.1f,
				{ 1.f, 0.f, 0.f, 1.f });*/
		}

		return Status::Success;
	}

private:
	EnemyBase* myEnemyBase = nullptr;

	float myShotTime = 3.f;
	float myShotCooldown = 5.f;
};

