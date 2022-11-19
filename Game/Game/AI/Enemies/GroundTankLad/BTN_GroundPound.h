#pragma once
#include "AI/BehaviorTree/Base/BT_Blackboard.h"
#include "AI/BehaviorTree/Base/BT_Leaf.h"
#include "Engine/GameObject/GameObject.h"
#include "Game/Components/HealthComponent.h"
#include "AI/Enemies/GroundTankLad/GroundTankLad.h"
#include "Engine/Shortcuts.h"

class BTN_GroundPound : public BT_Leaf
{
public:
	BTN_GroundPound(BT_Blackboard::Ptr aBlackBoard, EnemyBase* aEnemyBase) :
		BT_Leaf(aBlackBoard),
		myEnemyBase(aEnemyBase)
	{
	};

	Status update() override
	{
		if (myBlackboard->GetOwner()->GetComponent<GroundTankLad>()->CheckLaserStatus() == LaserBeam::eFireState::Ready && myBlackboard->GetOwner()->GetComponent<GroundTankLad>()->CheckPoundStatus() == GroundPound::eFireState::Ready)
		{
			myEnemyBase->RotateTowardsPlayer(16.f);

			myBlackboard->GetOwner()->GetComponent<GroundTankLad>()->StartPounding();
			if (myBlackboard->GetAnimationController()->HasTrigger("ActivateMelee"))
			{
				myBlackboard->GetAnimationController()->Trigger("ActivateMelee");
			}

			return Status::Success;
		}
		else
		{
			return Status::Failure;
		}
	}

private:
	EnemyBase* myEnemyBase = nullptr;
};

