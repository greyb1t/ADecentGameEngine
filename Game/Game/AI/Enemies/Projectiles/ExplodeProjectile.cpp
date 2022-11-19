#include "pch.h"
#include "ExplodeProjectile.h"

#include "AI/AIDirector/AIDirector.h"
#include "AI/Explosion/GeneralExplosion.h"
#include "AI/Health/ProjectileHealth.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "AI/VFX/EnemyVFXRef.h"
#include "Components/HealthComponent.h"
#include "Engine/Shortcuts.h"
#include "Engine/GameObject/Components/RigidBodyComponent.h"
#include "Engine/GameObject/Components/AudioComponent.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/VFX/VFXUtilities.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/GameObject/Components/DestructComponent.h"

ExplodeProjectile::ExplodeProjectile(GameObject* aGameObject, float aRadius, float aExplosionRadius) :
	Projectile(aGameObject),
	myRadius(aRadius),
	myExplosionRadius(aExplosionRadius)
{
}

void ExplodeProjectile::Start()
{
	Projectile::Start();
	myGameObject->SetName("ExplodeProjectile");
	myGameObject->SetTag(eTag::ENEMYPROJECTILE);

	auto obj = myGameObject->GetScene()->AddGameObject<GameObject>();
	obj->GetTransform().SetParent(&GetTransform());
	obj->GetTransform().SetPositionLocal({ 0,0,0 });

	const auto& vfxRef = myGameObject->GetSingletonComponent<EnemyVFXRef>()->GetAirTankProjectileTrail();
	myVFX = obj->AddComponent<Engine::VFXComponent>(vfxRef->Get());
	myVFX->Play();
	myVFX->AutoDestroy();

	Engine::GameObjectPrefab& enemySpawnPortalPrefab = myGameObject->GetSingletonComponent<AIDirector>()->GetAirTankProjectile()->Get();
	myModel = &enemySpawnPortalPrefab.Instantiate(*myGameObject->GetScene());
	myModel->GetTransform().SetPosition(myGameObject->GetTransform().GetPosition());

	myProjectileHealth = myGameObject->AddComponent<ProjectileHealth>();
	myProjectileHealth->Init(1.f, myExplosionRadius, 10.f);
	myProjectileHealth->SetDeathFunction([this]()
		{
			//myVFX->Stop();
			myModel->Destroy();

			SpawnExplosionSound();
			auto explosionObj = myGameObject->GetScene()->AddGameObject<GameObject>();
			explosionObj->GetTransform().SetPosition(myGameObject->GetTransform().GetPosition());
			const auto& vfxRef = myGameObject->GetSingletonComponent<EnemyVFXRef>()->GetAirTankProjectileExplode();
			explosionObj->AddComponent<GeneralExplosion>(vfxRef->Get());
		});

	auto rigidBody = myGameObject->AddComponent<Engine::RigidBodyComponent>(eRigidBodyType::KINEMATIC);
	rigidBody->Attach(Shape::Sphere(myRadius, ePhysicsMaterial::Default, true));
	myGameObject->SetLayers(eLayer::ENEMY_DAMAGEABLE, eLayer::NONE, eLayer::PLAYER);
	rigidBody->ObserveCollision([this](GameObject* obj)
		{
			OnCollision();
		});
}

void ExplodeProjectile::Execute(Engine::eEngineOrder aOrder)
{
	Projectile::Execute(aOrder);
	Update();
}

void ExplodeProjectile::Update()
{
	if (!CheckLifeTime())
	{
		myGameObject->Destroy();
		myModel->Destroy();
		//myVFX->Stop();
	}

	myModel->GetTransform().SetPosition(myGameObject->GetTransform().GetPosition());
	myModel->GetTransform().LookAt((myModel->GetTransform().GetPosition() + myDirection.GetNormalized() * 10000.f));
	myGameObject->GetTransform().LookAt((myGameObject->GetTransform().GetPosition() + myDirection.GetNormalized() * 10000.f));

	mySphereCastFrameCount++;
	if (mySphereCastFrameCount > mySphereCastFrameCooldown)
	{
		CreateSphereCast();
		mySphereCastFrameCount = 0;
	}

	/*RayCastHit hit;
	if (CheckCollisionAndMove(eLayer::MASK_BURSTPROJECTILE, hit, true))
	{
	}*/
	MoveInForwardAndRotateTowardsPlayer();


	GDebugDrawer->DrawSphere3D(
		DebugDrawFlags::AI,
		myGameObject->GetTransform().GetPosition(),
		5.f,
		0.f,
		{ 1.f, 0.f, 1.f, 1.f });
}

void ExplodeProjectile::Reflect(Engine::Reflector& aReflector)
{
	Projectile::Reflect(aReflector);
}

void ExplodeProjectile::Render()
{
	Projectile::Render();
}

void ExplodeProjectile::OnCollision()
{
	myProjectileHealth->GetHealthComponent()->ApplyDamage(10000000000.f);
	LOG_INFO(LogType::Game) << "BurstProjectile.cpp";
}

void ExplodeProjectile::CreateSphereCast()
{
	std::vector<OverlapHit> hits;
	myGameObject->GetScene()->SphereCastAll(myGameObject->GetTransform().GetPosition(), myRadius, eLayer::MASK_BURSTPROJECTILE | eLayer::PLAYER_SHIELD, hits);
	for (size_t i = 0; i < hits.size(); i++)
	{
		if (!hits[i].GameObject)
			continue;

		if (hits[i].GameObject->GetLayer() & eLayer::PLAYER_SHIELD)
		{
			ExplosionCreateSphereCast(false);
			continue;
		}

		if (hits[i].GameObject->GetTag() == eTag::ENEMYPROJECTILE)
			continue;

		if (hits[i].GameObject->GetTag() == eTag::ENEMY)
			continue;

		/*if (hits[i].GameObject->GetTag() == eTag::DEFAULT)
			continue;*/

		if (myShooterUUID == hits[i].GameObject->GetUUID())
			continue;

		ExplosionCreateSphereCast();

		SpawnExplosionSound();
		//if (hits[i].GameObject->GetLayer() & eLayer::PLAYER_DAMAGEABLE)
		//{
		//	auto health = hits[i].GameObject->GetComponent<HealthComponent>();

		//	if (health)
		//	{
		//		//health->ApplyDamage(myDamage);
		//	}
		//}
		//GDebugDrawer->DrawSphere3D(
		//	DebugDrawFlags::AI,
		//	myGameObject->GetTransform().GetPosition(),
		//	myExplosionRadius,
		//	0.5f,
		//	{ 1.f, 0.f, 0.f, 1.f });

		//myVFX->Stop();
		//myGameObject->Destroy();
		//myModel->Destroy();

		/*auto explosionObj = myGameObject->GetScene()->AddGameObject<GameObject>();
		explosionObj->GetTransform().SetPosition(myGameObject->GetTransform().GetPosition());
		const auto& vfxRef = myGameObject->GetSingletonComponent<EnemyVFXRef>()->GetAirTankProjectileExplode();
		explosionObj->AddComponent<GeneralExplosion>(vfxRef->Get());*/
	}
}

void ExplodeProjectile::ExplosionCreateSphereCast(bool aShouldDealDamage)
{
	std::vector<OverlapHit> hits;
	myGameObject->GetScene()->SphereCastAll(myGameObject->GetTransform().GetPosition(), myExplosionRadius, eLayer::MASK_BURSTPROJECTILE, hits);
	for (size_t i = 0; i < hits.size(); i++)
	{
		if (!hits[i].GameObject)
			continue;

		if (hits[i].GameObject->GetTag() == eTag::ENEMYPROJECTILE)
			continue;

		if (hits[i].GameObject->GetTag() == eTag::ENEMY)
			continue;

		/*if (hits[i].GameObject->GetTag() == eTag::DEFAULT)
			continue;*/

		if (myShooterUUID == hits[i].GameObject->GetUUID())
			continue;

		if (hits[i].GameObject->GetLayer() & eLayer::PLAYER_DAMAGEABLE)
		{
			auto health = hits[i].GameObject->GetComponent<HealthComponent>();

			if (health)
			{
				if (aShouldDealDamage)
					health->ApplyDamage(myDamage);
			}
		}
		GDebugDrawer->DrawSphere3D(
			DebugDrawFlags::AI,
			myGameObject->GetTransform().GetPosition(),
			myExplosionRadius,
			0.5f,
			{ 1.f, 0.f, 0.f, 1.f });

		//myVFX->Stop();
		myGameObject->Destroy();
		myModel->Destroy();

		auto explosionObj = myGameObject->GetScene()->AddGameObject<GameObject>();
		explosionObj->GetTransform().SetPosition(myGameObject->GetTransform().GetPosition());
		const auto& vfxRef = myGameObject->GetSingletonComponent<EnemyVFXRef>()->GetAirTankProjectileExplode();
		explosionObj->AddComponent<GeneralExplosion>(vfxRef->Get());
	}
}
void ExplodeProjectile::SpawnExplosionSound()
{
	auto gO = myGameObject->GetScene()->AddGameObject<GameObject>();
	gO->GetTransform().SetPosition(myGameObject->GetTransform().GetPosition());
	gO->AddComponent<Engine::DestructComponent>(5);

	auto audio = gO->AddComponent<Engine::AudioComponent>();
	audio->AddEvent("Explode", "event:/SFX/NPC/ProjectileExplode");
	audio->PlayEvent("Explode");
}

