#pragma once
#include "AI/BehaviorTree/Base/BT_Blackboard.h"
#include "AI/BehaviorTree/Base/BT_Leaf.h"
#include "Engine/GameObject/GameObject.h"

class BTN_InRangeOfPlayer : public BT_Leaf
{
public:
	BTN_InRangeOfPlayer(BT_Blackboard::Ptr aBlackBoard) :
		BT_Leaf(aBlackBoard)
	{};

	Status update() override
	{
		const Vec3f playerPos = 
			myBlackboard->GetOwner()->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos();

		const float distance = (playerPos - myBlackboard->GetOwner()->GetTransform().GetPosition()).Length();
		auto aswef = myBlackboard->getFloat("ShootingRange");

		if (distance < myBlackboard->getFloat("ShootingRange"))
		{
			return Status::Success;
		}
		return Status::Failure;
	}

private:

};

