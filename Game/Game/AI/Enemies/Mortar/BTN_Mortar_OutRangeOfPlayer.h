#pragma once
#include "AI/BehaviorTree/Base/BT_Blackboard.h"
#include "AI/BehaviorTree/Base/BT_Leaf.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "Engine/GameObject/GameObject.h"

class BTN_Mortar_OutRangeOfPlayer : public BT_Leaf
{
public:
	BTN_Mortar_OutRangeOfPlayer(BT_Blackboard::Ptr aBlackBoard) :
		BT_Leaf(aBlackBoard)
	{};

	Status update() override
	{
		const Vec3f playerPos =
			myBlackboard->GetOwner()->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos();

		const float distance = (playerPos - myBlackboard->GetOwner()->GetTransform().GetPosition()).Length();
		if (distance >= myBlackboard->getFloat("ShootingRange"))
		{
			if (myBlackboard->getBool("ExitShootRange"))
			{
				Enemy::SetEnemyCondition(*myBlackboard->GetAnimationController(), "InShootingRange", false);
				myBlackboard->setBool("ExitShootRange", false);
				myBlackboard->setBool("EnterShootRange", true);
			}
			return Status::Success;
		}
		return Status::Failure;
	}

private:

};

