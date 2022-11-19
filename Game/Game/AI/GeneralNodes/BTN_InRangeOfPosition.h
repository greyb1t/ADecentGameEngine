#pragma once
#include "AI/BehaviorTree/Base/BT_Blackboard.h"
#include "AI/BehaviorTree/Base/BT_Leaf.h"
#include "Engine/GameObject/GameObject.h"

class BTN_InRangeOfPosition : public BT_Leaf
{
public:
	BTN_InRangeOfPosition(BT_Blackboard::Ptr aBlackBoard, Vec3f aInRangePosition, float aRadius) :
		BT_Leaf(aBlackBoard),
		myInRangePosition(aInRangePosition),
		myRadius(aRadius)
	{};

	Status update() override
	{
		//const float distance = (myInRangePosition - myBlackboard->GetOwner()->GetTransform().GetPosition()).Length();
		const float distance = (myBlackboard->GetOwner()->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos() - myBlackboard->GetOwner()->GetTransform().GetPosition()).Length();
		if (distance < myRadius)
		{
			return Status::Success;
		}
		return Status::Failure;
	}

private:
	Vec3f myInRangePosition = { 0.f, 0.f, 0.f };
	float myRadius = 0.f;
};

