#pragma once
#include "AI/BehaviorTree/Base/BT_Blackboard.h"
#include "AI/BehaviorTree/Base/BT_Leaf.h"
#include "AI/Enemies/Base/EnemyBase.h"
#include "Engine/GameObject/GameObject.h"

class BTN_StayAboveGround : public BT_Leaf
{
public:
	BTN_StayAboveGround(BT_Blackboard::Ptr aBlackBoard, EnemyBase* aEnemyBase, float aCheckdistance) :
		BT_Leaf(aBlackBoard),
		myEnemyBase(aEnemyBase),
		myCheckDistance(aCheckdistance)
	{};

	Status update() override
	{
		auto rayCastPos = myBlackboard->GetOwner()->GetTransform().GetPosition();

		const auto rayCastDir = Vec3f(0.f, -1.f, 0.f);

		RayCastHit hit;
		if (myBlackboard->GetOwner()->GetScene()->RayCast(
			rayCastPos,
			rayCastDir,
			myCheckDistance,
			eLayer::NAVMESH,
			hit))

		{
			if (hit.GameObject != nullptr)
			{
				myMoveDir = myBlackboard->GetOwner()->GetTransform().GetPosition() - hit.Position;

				myMoveDir.Normalize();
				float speed = 100.f;
				const auto movement = myMoveDir * speed * Time::DeltaTime;
				myBlackboard->GetOwner()->GetTransform().Move(movement);

			}
		}



		return Status::Success;
	}

private:
	EnemyBase* myEnemyBase = nullptr;
	Vec3f myMoveDir = { 0.f, 0.f, 0.f };
	float myCheckDistance = 1000.f;
};

