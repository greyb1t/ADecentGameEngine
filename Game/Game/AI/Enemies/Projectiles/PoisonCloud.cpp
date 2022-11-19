#include "pch.h"
#include "PoisonCloud.h"

#include "Engine\Shortcuts.h"
#include "Engine/DebugManager/DebugDrawer.h"
#include "AI\Enemies\Base\EnemyBase.h"
#include "AI\PollingStation\PollingStationComponent.h"
#include "Components\HealthComponent.h"

PoisonCloud::PoisonCloud(GameObject* aGameObject) :
	Projectile(aGameObject)
{
}

PoisonCloud::~PoisonCloud()
{
}

void PoisonCloud::Init(EnemyBase* anEnemy, float aDamage, float aDuration, float aRadius)
{
	myEnemyRef = anEnemy;
	myDamage = aDamage;
	myDuration = aDuration;
	myRadius = aRadius;
}

void PoisonCloud::Execute(Engine::eEngineOrder aOrder)
{
	Projectile::Execute(aOrder);
	Update(Time::DeltaTime);
}

void PoisonCloud::Update(float aDeltaTime)
{
	if (myDurationTimer >= myDuration)
	{
		GetGameObject()->Destroy();
	}
	float temp = (myDurationTimer / myDuration);
	myCurrentRadius = myRadius * temp;
	GDebugDrawer->DrawSphere3D(
		DebugDrawFlags::Always,
		myGameObject->GetTransform().GetPosition(),
		myCurrentRadius,
		0.f,
		{ 1.f, 0.f, 1.f, 1.f });

	myDamageTimer += aDeltaTime;
	myDurationTimer += aDeltaTime;
	if (myDamageTimer < 1.f)
		return;

	std::vector<OverlapHit> hits;
	GetGameObject()->GetScene()->SphereCastAll(GetGameObject()->GetTransform().GetPosition(), myCurrentRadius, eLayer::PLAYER_DAMAGEABLE, hits);
	for (int hitIndex = 0; hitIndex < hits.size(); hitIndex++)
	{
		auto obj = hits[hitIndex].GameObject;
		if (!obj)
			continue;

		const float deltaY = abs(GetGameObject()->GetTransform().GetPosition().y - obj->GetTransform().GetPosition().y);
		if (deltaY <= myDamageHeight)
		{
			hits[hitIndex].GameObject->GetComponent<HealthComponent>()->ApplyDamage(myDamage, false, nullptr, eDamageType::Poison);
			myDamageTimer = 0;
		}
	}
	
}