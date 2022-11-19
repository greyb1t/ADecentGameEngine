#pragma once
#include "AI/BehaviorTree/Base/BT_Blackboard.h"
#include "AI/BehaviorTree/Base/BT_Leaf.h"
#include "Engine/GameObject/GameObject.h"

class BTN_InRangeOfMelee : public BT_Leaf
{
public:
	BTN_InRangeOfMelee(BT_Blackboard::Ptr aBlackBoard) :
		BT_Leaf(aBlackBoard)
	{};

	Status update() override
	{
		const Vec3f playerPos = 
			myBlackboard->GetOwner()->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos();

		const float distance = (playerPos - myBlackboard->GetOwner()->GetTransform().GetPosition()).Length();
		if (distance < myBlackboard->getFloat("MeleeRange"))
		{
			return Status::Success;
		}
		return Status::Failure;
	}

private:

};

