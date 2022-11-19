#pragma once
#include "AI/BehaviorTree/Base/BT_Blackboard.h"
#include "AI/BehaviorTree/Base/BT_Leaf.h"
#include "AI/Enemies/Base/EnemyBase.h"
#include "Engine/GameObject/GameObject.h"

class BTN_StupidWander : public BT_Leaf
{
public:
	BTN_StupidWander(BT_Blackboard::Ptr aBlackBoard, EnemyBase* aEnemyBase) :
		BT_Leaf(aBlackBoard),
		myEnemyBase(aEnemyBase)
	{};

	Status update() override
	{
		auto& transform = myBlackboard->GetOwner()->GetTransform();
		auto moveDir = transform.Forward() * -1.f;
		moveDir.Normalize();

		myNewRotationTimer += Time::DeltaTime;
		if (myNewRotationTimer > myNewRotationTime)
		{
			SetRotation();
			myNewRotationTimer = 0.f;
		}

		const float speed = myBlackboard->getFloat("MoveSpeed");
		const auto movement = moveDir * speed * Time::DeltaTime;
		myBlackboard->GetOwner()->GetTransform().Move(movement);

		return Status::Success;
	}

	void SetRotation()
	{
		const auto ownerPos = myBlackboard->GetOwner()->GetTransform().GetPosition();
		Vec3f targetPos;
		targetPos.x = ownerPos.x + Random::RandomFloat(-1000.f, 1000.f);
		targetPos.y = ownerPos.y;
		targetPos.z = ownerPos.z + Random::RandomFloat(-1000.f, 1000.f);

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

