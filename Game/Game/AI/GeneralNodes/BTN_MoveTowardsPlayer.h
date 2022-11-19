#pragma once
#include "AI/BehaviorTree/Base/BT_Blackboard.h"
#include "AI/BehaviorTree/Base/BT_Leaf.h"
#include "AI/Enemies/Base/EnemyBase.h"
#include "Engine/GameObject/GameObject.h"
#include "Components\HealthComponent.h"

class BTN_MoveTowardsPlayer : public BT_Leaf
{
public:
	BTN_MoveTowardsPlayer(BT_Blackboard::Ptr aBlackBoard, EnemyBase* aEnemyBase) :
		BT_Leaf(aBlackBoard),
		myEnemyBase(aEnemyBase)
	{};

	Status update() override
	{
		const Vec3f playerPos =
			myBlackboard->GetOwner()->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos();
		auto moveDir = playerPos - myBlackboard->GetOwner()->GetTransform().GetPosition();
		moveDir.Normalize();

		myEnemyBase->RotateTowardsPlayer(16.f);

		const float speed = myBlackboard->getFloat("Speed");
		const auto movement = moveDir * (speed - (speed * myEnemyBase->GetGameObject()->GetComponent<HealthComponent>()->GetSlowPercentage())) *
			Time::DeltaTime;
		myBlackboard->GetOwner()->GetTransform().Move(movement);

		myBlackboard->GetAnimationController()->SetFloat("Movement", speed);

		return Status::Success;
	}

private:
	EnemyBase* myEnemyBase = nullptr;

};

