#pragma once
#include "AI/BehaviorTree/Base/BT_Blackboard.h"
#include "AI/BehaviorTree/Base/BT_Leaf.h"
#include "AI/Enemies/Base/EnemyBase.h"
#include "Engine/GameObject/GameObject.h"

class BTN_MortarIdle : public BT_Leaf
{
public:
	BTN_MortarIdle(BT_Blackboard::Ptr aBlackBoard, EnemyBase* aEnemyBase) :
		BT_Leaf(aBlackBoard),
		myEnemyBase(aEnemyBase)
	{};

	Status update() override
	{
		return Status::Success;
	}

private:
	EnemyBase* myEnemyBase = nullptr;
};

