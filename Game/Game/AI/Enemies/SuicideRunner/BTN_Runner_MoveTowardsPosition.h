#pragma once
#include "AI/BehaviorTree/Base/BT_Blackboard.h"
#include "AI/BehaviorTree/Base/BT_Leaf.h"
#include "AI/Enemies/Base/EnemyBase.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "Engine/GameObject/GameObject.h"

class BTN_Runner_MoveTowardsPosition : public BT_Leaf
{
public:
	BTN_Runner_MoveTowardsPosition(BT_Blackboard::Ptr aBlackBoard, Vec3f aPositionToMoveTowards, EnemyBase* aEnemyBase, std::function<void()> aFunctional = nullptr) :
		BT_Leaf(aBlackBoard),
		myPositionToMoveTowards(aPositionToMoveTowards),
		myEnemyBase(aEnemyBase),
		myFunctional(aFunctional)
	{};

	Status update() override
	{
		if (!myBlackboard->getBool("CanStartRun"))
			return Status::Failure;

		const float distance = (myBlackboard->GetOwner()->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos() -
			myBlackboard->GetOwner()->GetTransform().GetPosition()).Length();

		if (distance < 300.f)
		{
			myFunctional();
			return Status::Failure;
		}

		auto rayPos = myBlackboard->GetOwner()->GetTransform().GetPosition();
		rayPos.y += 50.f;
		const auto rayCastDir = myBlackboard->GetOwner()->GetTransform().Forward().GetNormalized() * -1.f;
		const float rayLength = 300.f;
		RayCastHit hit;
		if (myBlackboard->GetOwner()->GetScene()->RayCast(
			rayPos,
			rayCastDir,
			rayLength,
			eLayer::GROUND | eLayer::DEFAULT | eLayer::PLAYER_SHIELD,
			hit))
		{
			if (hit.GameObject != nullptr)
			{
				if (myFunctional != nullptr)
				{
					myFunctional();
				}
			}
		}
		GDebugDrawer->DrawLine3D(
			DebugDrawFlags::AI,
			rayPos,
			rayPos + rayCastDir * rayLength,
			0.f,
			{ 1.f, 0.f, 1.f, 1.f });

		BackUpExplosion();

		if (myBlackboard->GetAnimationController() && myBlackboard->GetAnimationController()->HasBool("IsRunning"))
			Enemy::SetEnemyCondition(*myBlackboard->GetAnimationController(), "IsRunning", true);

		auto moveDir = myPositionToMoveTowards - myBlackboard->GetOwner()->GetTransform().GetPosition();
		moveDir.Normalize();

		//myEnemyBase->RotateTowardsPlayer(16.f);
		SetRotation();

		auto dir = myBlackboard->GetOwner()->GetTransform().Forward() * -1.f;
		dir.Normalize();

		const auto movement = /*moveDir*/dir * myBlackboard->getFloat("Speed") * Time::DeltaTime;
		myBlackboard->GetOwner()->GetTransform().Move(movement);


		return Status::Success;
	}

	void BackUpExplosion()
	{
		myBackUpExplosionTimer += Time::DeltaTime;
		if (myBackUpExplosionTimer > myBackUpExplosionTime)
		{
			myFunctional();
		}

	};

	void SetRotation()
	{
		if (const auto pollingStationComp = myBlackboard->GetOwner()->GetSingletonComponent<PollingStationComponent>())
		{
			const auto playerPos = pollingStationComp->GetPlayerPos();
			const auto ownerPos = myBlackboard->GetOwner()->GetTransform().GetPosition();

			auto moveDir = playerPos - ownerPos;
			moveDir.Normalize();

			float angle = atan2(moveDir.z, moveDir.x);
			myEnemyBase->SetTargetRotationRadians(angle + 90.f * Math::Deg2Rad, 4.f);
		}
		else
		{
			const auto ownerPos = myBlackboard->GetOwner()->GetTransform().GetPosition();
			Vec3f targetPos = myPositionToMoveTowards;

			auto moveDir = targetPos - ownerPos;
			moveDir.Normalize();

			float angle = atan2(moveDir.z, moveDir.x);
			myEnemyBase->SetTargetRotationRadians(angle + 90.f * Math::Deg2Rad, 4.f);
		}

		//Vec3f targetPos = myBlackboard->getVec3f("SuicidePosition");


	}

private:
	Vec3f myPositionToMoveTowards = { 0.f, 0.f, 0.f };
	EnemyBase* myEnemyBase = nullptr;
	std::function<void()> myFunctional = nullptr;

	float myBackUpExplosionTimer = 0.f;
	float myBackUpExplosionTime = 5.f;
};

