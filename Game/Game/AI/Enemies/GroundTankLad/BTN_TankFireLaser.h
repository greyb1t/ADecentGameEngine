#pragma once
#include "AI/BehaviorTree/Base/BT_Blackboard.h"
#include "AI/BehaviorTree/Base/BT_Leaf.h"
#include "Engine/GameObject/GameObject.h"
#include "Game/Components/HealthComponent.h"
#include "AI/Enemies/GroundTankLad/GroundTankLad.h"
#include "AI/Enemies/Testing/LaserTest.h"
#include "Engine/Shortcuts.h"

class BTN_TankFireLaser : public BT_Leaf
{
public:
	BTN_TankFireLaser(BT_Blackboard::Ptr aBlackBoard, GameObject* aLaser, float aShotCooldown, float aStartCoolDown) :
		BT_Leaf(aBlackBoard),
		myLaser(aLaser),
		myShootCooldown(aShotCooldown),
		myShootTimer(aStartCoolDown)
	{};

	Status update() override
	{
		myBlackboard->GetOwner()->GetComponent<GroundTankLad>()->RotateTowardsPlayer(16);


		if (myBlackboard->GetOwner()->GetComponent<GroundTankLad>()->CheckLaserStatus() == LaserBeam::eFireState::Ready)
		{
			myShootTimer += Time::DeltaTime;
			if (myShootTimer > myShootCooldown)
			{
				myLaser->GetComponent<LaserTest>()->ActivateLaser();
				myShootTimer = 0.f;
			}

			//myBlackboard->GetOwner()->GetComponent<GroundTankLad>()->IgniteLaser();
			return Status::Success;
		}
		else
		{
			return Status::Failure;
		}
	}

private:
	GameObject* myLaser = nullptr;
	float myShootTimer = 2.f;
	float myShootCooldown = 3.5f;
};

