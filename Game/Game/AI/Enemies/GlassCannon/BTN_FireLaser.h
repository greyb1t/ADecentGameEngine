#pragma once
#include "AI/BehaviorTree/Base/BT_Blackboard.h"
#include "AI/BehaviorTree/Base/BT_Leaf.h"
#include "Engine/GameObject/GameObject.h"
#include "Game/Components/HealthComponent.h"
#include "AI/Enemies/GlassCannon/GlassCannon.h"
#include "Engine/Shortcuts.h"

class BTN_FireLaser : public BT_Leaf
{
public:
	BTN_FireLaser(BT_Blackboard::Ptr aBlackBoard) :
		BT_Leaf(aBlackBoard)
	{};

	Status update() override
	{
	
		myBlackboard->GetOwner()->GetComponent<GlassCannon>()->RotateTowardsPlayer(16);


		if (myBlackboard->GetOwner()->GetComponent<GlassCannon>()->CheckLaserStatus() == LaserBeam::eFireState::Ready)
		{
			myBlackboard->GetOwner()->GetComponent<GlassCannon>()->IgniteLaser();
			return Status::Success;
		}
		else
		{
			return Status::Failure;
		}
	}

private:
};

