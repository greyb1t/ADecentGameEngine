#pragma once
#include "AI/BehaviorTree/Base/BT_Blackboard.h"
#include "AI/BehaviorTree/Base/BT_Leaf.h"
#include "AI/Enemies/Base/EnemyBase.h"
#include "Engine/GameObject/GameObject.h"

class BTN_MoveTowardsPosition : public BT_Leaf
{
public:
	BTN_MoveTowardsPosition(BT_Blackboard::Ptr aBlackBoard, Vec3f aPositionToMoveTowards, EnemyBase* aEnemyBase) :
		BT_Leaf(aBlackBoard),
		myPositionToMoveTowards(aPositionToMoveTowards),
		myEnemyBase(aEnemyBase)
	{};

	Status update() override
	{
		auto moveDir = myPositionToMoveTowards - myBlackboard->GetOwner()->GetTransform().GetPosition();
		moveDir.Normalize();

		myEnemyBase->RotateTowardsPlayer(16.f);

		const auto movement = moveDir * myBlackboard->getFloat("Speed") * Time::DeltaTime;
		myBlackboard->GetOwner()->GetTransform().Move(movement);

		return Status::Success;
	}

private:
	Vec3f myPositionToMoveTowards = { 0.f, 0.f, 0.f };
	EnemyBase* myEnemyBase = nullptr;

};

