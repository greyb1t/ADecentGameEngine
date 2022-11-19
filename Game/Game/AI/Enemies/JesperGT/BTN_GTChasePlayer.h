#pragma once
#include "AI/BehaviorTree/Base/BT_Blackboard.h"
#include "AI/BehaviorTree/Base/BT_Leaf.h"
#include "AI/Enemies/Base/EnemyBase.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "Engine/GameObject/GameObject.h"

class BTN_GTChasePlayer : public BT_Leaf
{
public:
	BTN_GTChasePlayer(BT_Blackboard::Ptr aBlackBoard, EnemyBase* aEnemyBase) :
		BT_Leaf(aBlackBoard),
		myEnemyBase(aEnemyBase)
	{};

	Status update() override
	{
		Enemy::SetEnemyCondition(*myBlackboard->GetAnimationController(), "Walking", true);

		auto moveDir = myBlackboard->GetOwner()->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos() -
			myBlackboard->GetOwner()->GetTransform().GetPosition();
		moveDir.Normalize();

		SetRotation();

		const float speed = myBlackboard->getFloat("MoveSpeed");
		const auto movement = moveDir * speed * Time::DeltaTime;
		myBlackboard->GetOwner()->GetTransform().Move(movement);

		return Status::Success;
	}

	void SetRotation()
	{
		const auto ownerPos = myBlackboard->GetOwner()->GetTransform().GetPosition();
		Vec3f targetPos = myBlackboard->GetOwner()->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos();

		auto moveDir = targetPos - ownerPos;
		moveDir.Normalize();

		float angle = atan2(moveDir.z, moveDir.x);
		myEnemyBase->SetTargetRotationRadians(angle + 90.f * Math::Deg2Rad, 16.f);
	}

private:
	EnemyBase* myEnemyBase = nullptr;
	float myNewRotationTimer = 1.5f;
	float myNewRotationTime = 1.5f;
};

