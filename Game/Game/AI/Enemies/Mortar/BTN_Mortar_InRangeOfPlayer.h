#pragma once
#include "AI/BehaviorTree/Base/BT_Blackboard.h"
#include "AI/BehaviorTree/Base/BT_Leaf.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "Engine/GameObject/GameObject.h"

class BTN_Mortar_InRangeOfPlayer : public BT_Leaf
{
public:
	BTN_Mortar_InRangeOfPlayer(BT_Blackboard::Ptr aBlackBoard) :
		BT_Leaf(aBlackBoard)
	{};

	Status update() override
	{
		const Vec3f playerPos =
			myBlackboard->GetOwner()->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos();

		const float distance = (playerPos - myBlackboard->GetOwner()->GetTransform().GetPosition()).Length();
		if (distance < myBlackboard->getFloat("ShootingRange"))
		{
			if (myBlackboard->getBool("EnterShootRange"))
			{
				Enemy::SetEnemyCondition(*myBlackboard->GetAnimationController(), "InShootingRange", true);
				myBlackboard->setBool("EnterShootRange", false); 
				myBlackboard->setBool("ExitShootRange", true); 
			}
			return Status::Success;
		}
		return Status::Failure;
	}

private:

};

