#include "pch.h"
#include "ExplosionComponent.h"
#include "Engine/Reflection/Reflector.h"
#include "Engine/GameObject/GameObject.h"

#include "Game/Components/HealthComponent.h"
#include "Engine\GameObject\Components/RigidBodyComponent.h"

#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/GameObject/Components/VFXComponent.h"

#include <vector>

ExplosionComponent::ExplosionComponent(GameObject* aGameObject)
{
	myGameObject = aGameObject;
}

ExplosionComponent::~ExplosionComponent()
{
}

void ExplosionComponent::Start()
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

	myModelChild = GetTransform().GetChildren()[0]->GetGameObject();

	myGameObject->SetLayer(eLayer::ENVIRONMENT_DAMAGEABLE);
	Explode();
}
void ExplosionComponent::Execute(Engine::eEngineOrder aOrder)
{
	if (myHasExploded)
	{
		myDeathTimer += Time::DeltaTime;
		if (myDeathTimer > myDeathTime)
		{
			myGameObject->Destroy();
			//myGameObject->GetComponent<Engine::RigidBodyComponent>()->Destroy();
			myModelChild->Destroy();
		}
	}
}
void ExplosionComponent::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myExplosionRef, "Explosion VFX");
	aReflector.Reflect(myExplosionRadius, "Explosion Radius");
	aReflector.Reflect(myDamage, "Damage");
}
void ExplosionComponent::Explode()
{
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
		
		if (gameObject->GetComponent<HealthComponent>())
		{
			gameObject->GetComponent<HealthComponent>()->ApplyDamage(myDamage);
		}
	}

	if (myExplosionVFX)
		myExplosionVFX->Play();

	myHasExploded = true;
}