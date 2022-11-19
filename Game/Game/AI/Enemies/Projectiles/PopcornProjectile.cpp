#include "pch.h"
#include "PopcornProjectile.h"

#include "AI/AIDirector/AIDirector.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "AI/VFX/EnemyVFXRef.h"
#include "Components/HealthComponent.h"
#include "Engine/Shortcuts.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/VFX/VFXUtilities.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"

PopcornProjectile::PopcornProjectile(GameObject* aGameObject) :
	Projectile(aGameObject)
{
}

void PopcornProjectile::Start()
{
	Projectile::Start();
	auto obj = myGameObject->GetScene()->AddGameObject<GameObject>();
	obj->GetTransform().SetParent(&GetTransform());
	obj->GetTransform().SetPositionLocal(Vec3f(0.f, 0.f, 0.f));

	const auto& vfxRef = myGameObject->GetSingletonComponent<EnemyVFXRef>()->GetAirPopProjectileTrail();
	myVFX = obj->AddComponent<Engine::VFXComponent>(vfxRef->Get());
	myVFX->Play();
	myVFX->AutoDestroy();

	/*myVFX = obj->AddComponent<Engine::VFXComponent>(VFX::Load("Assets/VFX/Fireball.vfx"));
	myVFX->Play();
	myVFX->AutoDestroy();*/

	Engine::GameObjectPrefab& popCornProjectile = myGameObject->GetSingletonComponent<AIDirector>()->GetPopCornProjectile()->Get();
	myModel = &popCornProjectile.Instantiate(*myGameObject->GetScene());
	myModel->GetTransform().SetPosition(myGameObject->GetTransform().GetPosition());
}

void PopcornProjectile::Execute(Engine::eEngineOrder aOrder)
{
	Projectile::Execute(aOrder);
	Update();
}

void PopcornProjectile::Update()
{
	if (!CheckLifeTime())
	{
		myGameObject->Destroy();
		myModel->Destroy();

		myVFX->Stop();
	}

	mySphereCastFrameCount++;
	if (mySphereCastFrameCount > mySphereCastFrameCooldown)
	{
		CreateSphereCast();
		mySphereCastFrameCount = 0;
	}

	myModel->GetTransform().SetPosition(myGameObject->GetTransform().GetPosition());
	myModel->GetTransform().LookAt((myModel->GetTransform().GetPosition() + myDirection.GetNormalized() * 10000.f));
	myGameObject->GetTransform().LookAt((myGameObject->GetTransform().GetPosition() + myDirection.GetNormalized() * 10000.f));

	RayCastHit hit;
	if (CheckCollisionAndMove(eLayer::SHOTOBSTACLE, hit, false))
	{
		if (hit.GameObject && hit.GameObject->GetLayer() & eLayer::PLAYER)
		{
			if (hit.GameObject)
			{
				myVFX->Stop();
				myGameObject->Destroy();
				myModel->Destroy();
			}
		}
	}
}

void PopcornProjectile::Reflect(Engine::Reflector& aReflector)
{
	Projectile::Reflect(aReflector);
}

void PopcornProjectile::Render()
{
	Projectile::Render();
}

void PopcornProjectile::CreateSphereCast() const
{
	constexpr float radius = 50.f;
	std::vector<OverlapHit> hits;
	myGameObject->GetScene()->SphereCastAll(myGameObject->GetTransform().GetPosition(), radius, eLayer::PLAYER_DAMAGEABLE | eLayer::PLAYER_SHIELD, hits);
	for (size_t i = 0; i < hits.size(); i++)
	{
		if (!hits[i].GameObject)
			continue;

		if (hits[i].GameObject->GetLayer() & eLayer::PLAYER_SHIELD)
		{
			myVFX->Stop();
			myGameObject->Destroy();
			myModel->Destroy();
		}

		if (myShooterUUID == hits[i].GameObject->GetUUID())
			continue;

		if (hits[i].GameObject->GetLayer() & eLayer::PLAYER_DAMAGEABLE)
		{
			if (const auto health = hits[i].GameObject->GetComponent<HealthComponent>())
			{
				health->ApplyDamage(myDamage);
			}
		}

		myVFX->Stop();
		myGameObject->Destroy();
		myModel->Destroy();
	}
}