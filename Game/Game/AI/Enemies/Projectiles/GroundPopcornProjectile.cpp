#include "pch.h"
#include "GroundPopcornProjectile.h"

#include "Components/HealthComponent.h"
#include "Engine/Shortcuts.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/VFX/VFXUtilities.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "AI/VFX/EnemyVFXRef.h"
#include "Engine/GameObject/Tag.h"

GroundPopcornProjectile::GroundPopcornProjectile(GameObject* aGameObject) :
	Projectile(aGameObject)
{
}

void GroundPopcornProjectile::Start()
{
	Projectile::Start();

	auto obj = myGameObject->GetScene()->AddGameObject<GameObject>();
	obj->GetTransform().SetParent(&GetTransform());
	obj->GetTransform().SetPositionLocal({ 0,0,0 });

	const auto& vfxRef = myGameObject->GetSingletonComponent<EnemyVFXRef>()->GetGroundPopcornProjectile();
	myVFX = GetGameObject()->AddComponent<Engine::VFXComponent>(vfxRef->Get());
	myVFX->Play();
	myVFX->AutoDestroy();
}

void GroundPopcornProjectile::Execute(Engine::eEngineOrder aOrder)
{
	Projectile::Execute(aOrder);
	Update();
}

void GroundPopcornProjectile::Update()
{
	if (!CheckLifeTime())
	{
		myGameObject->Destroy();
		Destroy();
	}

	//CreateSphereCast();

	Move();

	mySphereCastFrameCount++;
	if (mySphereCastFrameCount > mySphereCastFrameCooldown)
	{
		CreateSphereCast2();
		mySphereCastFrameCount = 0;
	}
}

void GroundPopcornProjectile::SetRadius(float aRadius)
{
	myRadius = aRadius;
}

void GroundPopcornProjectile::Reflect(Engine::Reflector& aReflector)
{
	Projectile::Reflect(aReflector);
}

void GroundPopcornProjectile::Render()
{
	Projectile::Render();
}

void GroundPopcornProjectile::CreateSphereCast()
{
	/*static std::vector<OverlapHit> hits;
	hits.clear();

	Vec3f position = myGameObject->GetTransform().GetPosition();
 	const float distance = mySpeed * Time::DeltaTime;
	position += myDirection * distance;
	myGameObject->GetTransform().SetPosition(position);

	if (myGameObject->GetScene()->SphereCastAll(position, myRadius, eLayer::MASK_BURSTPROJECTILE, hits) == true)
	{
		for (int hitIndex = 0; hitIndex < hits.size(); hitIndex++)
		{
			if (hits[hitIndex].GameObject->GetTag() == eTag::ENEMY)
				continue;

			if (hits[hitIndex].GameObject != nullptr)
			{
				myHitPos = position;

				for (int hitIndex = 0; hitIndex < hits.size(); hitIndex++)
				{
					if (hits[hitIndex].GameObject && hits[hitIndex].GameObject->GetLayer() & eLayer::PLAYER_DAMAGEABLE | eLayer::PLAYER_SHIELD)
					{
						if (hits[hitIndex].GameObject)
						{
							const auto health = hits[hitIndex].GameObject->GetComponent<HealthComponent>();

							if (health != nullptr)
							{
								health->ApplyDamage(myDamage);
							}

							myGameObject->Destroy();
							myVFX->Destroy();
							Destroy();
						}
					}
				}
			}
		}
	}*/
}

void GroundPopcornProjectile::CreateSphereCast2()
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
			myVFX->Destroy();
			//myVFX->Stop();
			myGameObject->Destroy();
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

		myVFX->Destroy();
		//myVFX->Stop();
		myGameObject->Destroy();
	}
}

void GroundPopcornProjectile::Move()
{
	RayCastHit hit;
	if (CheckCollisionAndMove(eLayer::SHOTOBSTACLE, hit, false))
	{
		if (hit.GameObject && hit.GameObject->GetLayer() & eLayer::PLAYER)
		{
			if (hit.GameObject)
			{
				//myVFX->Stop();
				myVFX->Destroy();
				myGameObject->Destroy();
			}
		}
	}
}

