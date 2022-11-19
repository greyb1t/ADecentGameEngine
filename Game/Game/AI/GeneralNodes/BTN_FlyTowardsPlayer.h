#pragma once
#include "AI/BehaviorTree/Base/BT_Blackboard.h"
#include "AI/BehaviorTree/Base/BT_Leaf.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "Engine/GameObject/GameObject.h"

class BTN_FlyTowardsPlayer : public BT_Leaf
{
public:
	BTN_FlyTowardsPlayer(BT_Blackboard::Ptr aBlackBoard, EnemyBase* aEnemyBase) :
		BT_Leaf(aBlackBoard),
		myEnemyBase(aEnemyBase)
	{};

	Status update() override
	{
		const auto  playerPos = myEnemyBase->GetGameObject()->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos();


		auto flyDir = playerPos - myBlackboard->GetOwner()->GetTransform().GetPosition();
		flyDir.Normalize();

		myEnemyBase->RotateTowardsPlayer(16.f);

		float speed = 100.f;
		auto movement = flyDir * speed * Time::DeltaTime;
		myBlackboard->GetOwner()->GetTransform().Move(movement);

		return Status::Success;
	}

private:
	EnemyBase* myEnemyBase = nullptr;
};

