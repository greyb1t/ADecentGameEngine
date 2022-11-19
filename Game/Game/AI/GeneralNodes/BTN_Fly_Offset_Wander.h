#pragma once
#include "AI/BehaviorTree/Base/BT_Blackboard.h"
#include "AI/BehaviorTree/Base/BT_Leaf.h"
#include "AI/Enemies/Base/EnemyBase.h"
#include "Engine/GameObject/GameObject.h"

class BTN_Fly_Offset_Wander : public BT_Leaf
{
public:
	BTN_Fly_Offset_Wander(BT_Blackboard::Ptr aBlackBoard, EnemyBase* aEnemyBase, float aSpeed) :
		BT_Leaf(aBlackBoard),
		myEnemyBase(aEnemyBase),
		mySpeed(aSpeed)
	{};

	Status update() override
	{
		myTimer += Time::DeltaTime;
		if (myTimer > myTime)
		{
			myMoveDir = Random::InsideUnitSphere();
			myMoveDir.Normalize();
			myTimer = 0.f;
		}

		myEnemyBase->RotateTowardsPlayer(16.f);

		const auto movement = myMoveDir * mySpeed * Time::DeltaTime;
		myBlackboard->GetOwner()->GetTransform().Move(movement);

		return Status::Success;
	}

private:
	EnemyBase* myEnemyBase = nullptr;
	Vec3f myMoveDir = { 0.f, 0.f, 0.f };
	float myTimer = 10000.f;
	float myTime = 0.5f;
	float mySpeed = 50.f;
};

