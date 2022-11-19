#include "pch.h"
#include "ExplosiveObjectComponent.h"
#include "Engine/Reflection/Reflector.h"
#include "Engine/GameObject/GameObject.h"

#include "Game/Components/HealthComponent.h"
#include "Engine\GameObject\Components/RigidBodyComponent.h"

#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/GameObject/Components/VFXComponent.h"

#include <vector>

ExplosiveObjectComponent::ExplosiveObjectComponent(GameObject* aGameObject)
{
	myGameObject = aGameObject;
}

ExplosiveObjectComponent::~ExplosiveObjectComponent()
{
}

void ExplosiveObjectComponent::Start()
{
	{
		if (myExplosionRef->IsValid())
		{
			myExplosionVFX = myGameObject->AddComponent<Engine::VFXComponent>(myExplosionRef->Get());
		}
		else
		{
			LOG_ERROR(LogType::Game) << "Explosion barrel is missing VFX";
		}
	}

	{
		myHealthComponent = myGameObject->GetComponent<HealthComponent>();

		if (!myHealthComponent)
			myHealthComponent = myGameObject->AddComponent<HealthComponent>();

		myHealthComponent->SetMaxHealth(myMaxHealth);

		myHealthComponent->ObserveDeath([&](float aDmg) { OnDeath(); });
	}

	myModelChild = GetTransform().GetChildren()[0]->GetGameObject();

	myGameObject->SetLayer(eLayer::ENVIRONMENT_DAMAGEABLE);
}

void ExplosiveObjectComponent::Setup(const float& aHealth)
{
	myHealthComponent->SetHealth(aHealth);
}

void ExplosiveObjectComponent::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myExplosionRef, "Explosion VFX");
	aReflector.Reflect(myExplosionRadius, "Explosion Radius");
	aReflector.Reflect(myDamage, "Damage");
	aReflector.Reflect(myMaxHealth, "Max Health");
}

void ExplosiveObjectComponent::OnDeath()
{
	if (myHasPlayedDeath)
		return;
	
	myHasPlayedDeath = true;


	//{ //Till när simon vill testa :^)
	//	ItemEventData data;
	//	data.targetObject = myGameObject;
	//	data.damage = 0.0f;
	//	Main::GetItemManager().PostEvent(eItemEvent::OnEnemyKill, &data);
	//}


	GDebugDrawer->DrawSphere3D(DebugDrawFlags::Gameplay, GetTransform().GetPosition(), myExplosionRadius, 1.0f);

	std::vector<OverlapHit> outHits;
	GetGameObject()->GetScene()->SphereCastAll(GetTransform().GetPosition(), myExplosionRadius, eLayer::ALL, outHits);

	for (auto& hit : outHits)
	{
		auto gameObject = hit.GameObject;
		if (!gameObject)
			continue;

		if (gameObject->GetUUID() == myGameObject->GetUUID())
			continue;

		auto healthComp = gameObject->GetComponent<HealthComponent>();
		if (!healthComp)
			continue;

		healthComp->ApplyDamage(myDamage);
	}


	if (myExplosionVFX)
		myExplosionVFX->Play();

	myModelChild->Destroy();
	myGameObject->GetComponent<Engine::RigidBodyComponent>()->Destroy();
}
