#include "pch.h"
#include "PoisonCloudDamageComponent.h"
#include "Engine/GameObject/GameObject.h"
#include <AI/PollingStation/PollingStationComponent.h>
#include "Game/Components/HealthComponent.h"

PoisonCloudDamageComponent::PoisonCloudDamageComponent(GameObject* anObject)
	: Component(anObject)
{

}

void PoisonCloudDamageComponent::Execute(Engine::eEngineOrder aOrder)
{
	myDuration += Time::DeltaTime;

	if (myDuration > myLifeTime)
	{
		GetGameObject()->Destroy();
		return;
	}

	auto* pollingStation = GetGameObject()->GetSingletonComponent<PollingStationComponent>();
	if (!pollingStation)
		return;

	float distSqr = (pollingStation->GetPlayerPos() - GetTransform().GetPosition()).LengthSqr();
	if (distSqr < myDmgDist * myDmgDist)
	{
		myTickProgress += Time::DeltaTime;
		if (myTickProgress > 1.f / myTicksPerSecond)
		{
			myTickProgress = 0.0f;
			auto* player = pollingStation->GetPlayer();
			if (!player)
				return;

			auto* health = player->GetComponent<HealthComponent>();
			if (!health)
				return;

			health->ApplyDamage(myTickDamage);
		}
	}

	GDebugDrawer->DrawSphere3D(DebugDrawFlags::Always, GetTransform().GetPosition(), myDmgDist);
}