#pragma once
#include "AI/BehaviorTree/Base/BT_Blackboard.h"
#include "AI/BehaviorTree/Base/BT_Leaf.h"
#include "Engine/GameObject/GameObject.h"
#include "Game/Components/HealthComponent.h"
#include "Engine/Shortcuts.h"

class BTN_SuicideExplode : public BT_Leaf
{
public:
	BTN_SuicideExplode(BT_Blackboard::Ptr aBlackBoard) :
		BT_Leaf(aBlackBoard)
	{};

	Status update() override
	{
		const Vec3f ownerPos = myBlackboard->GetOwner()->GetTransform().GetPosition();

		GDebugDrawer->DrawSphere3D(
			DebugDrawFlags::AI,
			ownerPos,
			200.f,
			0.5f,
			{ 1.f, 0.f, 0.f, 1.f });

		std::vector<OverlapHit> hits;
		myBlackboard->GetOwner()->GetScene()->SphereCastAll(ownerPos, 200.f, eLayer::PLAYER, hits);
		for (size_t i = 0; i < hits.size(); i++)
		{
			if (!hits[i].GameObject)
				continue;

			auto health = hits[i].GameObject->GetComponent<HealthComponent>();

			if (health)
			{
				health->ApplyDamage(10.f);
			}
		}

		myBlackboard->GetOwner()->Destroy();
		return Status::Success;
	}

private:
};

