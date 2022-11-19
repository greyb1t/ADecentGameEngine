#include "pch.h"
#include "FinalBossArenaTrigger.h"
#include "Engine/Scene/Scene.h"
#include "Engine/GameObject/Components/RigidBodyComponent.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/TriggerEventManager/TriggerEventManager.h"
#include "AI/PollingStation/PollingStationComponent.h"

void FB::FinalBossArenaTrigger::Start()
{
}

void FB::FinalBossArenaTrigger::Execute(Engine::eEngineOrder aOrder)
{
	std::vector<OverlapHit> hits;

	auto rb = myGameObject->GetComponent<Engine::RigidBodyComponent>();

	if (!myHasPlayerEnteredTrigger)
	{
		// NOTE(filip): Must do box cast/sphere cast because the trigger does not work
		const bool hitSomething = myGameObject->GetScene()->SphereCastAll(
			rb->GetTransform().GetPosition(),
			rb->GetShapes()[0].GetRadius(),
			rb->GetGameObject()->GetDetectionLayerMask(),
			hits);

		if (hitSomething)
		{
			for (int i = static_cast<int>(hits.size()) - 1; i >= 0; --i)
			{
				const auto& hit = hits[i];

				auto polling = myGameObject->GetSingletonComponent<PollingStationComponent>();

				const bool isPlayerInUpperHalfOfSphere = polling->GetPlayerPos().y >= GetTransform().GetPosition().y;

				if (hit.GameObject->GetTag() == eTag::PLAYER && isPlayerInUpperHalfOfSphere)
				{
					TriggerEventManager::GetInstance().SendLetter("EnteredBossArena", nullptr);

					myHasPlayerEnteredTrigger = true;

					// we dont need it anymore
					myGameObject->Destroy();
				}
			}
		}
	}
}

void FB::FinalBossArenaTrigger::Reflect(Engine::Reflector& aReflector)
{
}