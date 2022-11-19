#include "pch.h"
#include "MortarProjectile.h"

#include "AI/AIDirector/AIDirector.h"
#include "AI/Explosion/GeneralExplosion.h"
#include "AI/Health/ProjectileHealth.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "AI/VFX/EnemyVFXRef.h"
#include "Components/HealthComponent.h"
#include "Engine/Shortcuts.h"
#include "Engine/GameObject/Components/DecalComponent.h"
#include "Engine/GameObject/Components/RigidBodyComponent.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"
#include "Engine/ResourceManagement/Resources/MaterialResource.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/VFX/VFXUtilities.h"
#include "Player/Player.h"

MortarProjectile::MortarProjectile(GameObject* aGameObject, float aProjectileRadius, float aExplosionRadius) :
	Projectile(aGameObject),
	myRadius(aProjectileRadius),
	myExplosionRadius(aExplosionRadius)
{
}

void MortarProjectile::Start()
{
	Projectile::Start();
	myGameObject->SetName("MortarProjectile");
	myGameObject->SetTag(eTag::ENEMYPROJECTILE);

	const auto playerComp = dynamic_cast<Player*>(myGameObject->GetSingletonComponent<PollingStationComponent>()->GetPlayerComponent());
	const float playerMoveDiry = playerComp->GetVelocity().Get().y;
	Vec3f playerMoveDir = playerComp->GetStatus().movementDirection;
	playerMoveDir.y = playerMoveDiry;
	playerMoveDir.Normalize();

	const float distance = Random::RandomFloat(0.f, 1500.f);

	const Vec3f rayPos = myGameObject->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos() + playerMoveDir * distance;

	{
		RayCastHit hit;
		if (myGameObject->GetScene()->RayCast(rayPos,
			Vec3f(0.f, -1.f, 0.f),
			10000.f,
			eLayer::DEFAULT,
			hit))
		{
			if (hit.GameObject != nullptr)
			{
				Engine::GameObjectPrefab& indicator = myGameObject->GetSingletonComponent<AIDirector>()->GetMortarIndicator()->Get();
				myindicator = &indicator.Instantiate(*myGameObject->GetScene());
				myindicator->GetTransform().SetPosition(hit.Position);

				{ // Decal
					//myBulletHitDecal = GResourceManager->CreateRef<Engine::GameObjectPrefabResource>("Assets/Prefabs/MortarDecal.prefabg");
					//myBulletHitDecal->Load();

					//myMatRef = GResourceManager->CreateRef<Engine::MaterialResource>("Assets/Materials/Decal/TestDecal.decalmat");
					////myMatRef = GResourceManager->CreateRef<Engine::MaterialResource>("Assets/Engine/Materials/BulletHoleDecalPS.material");
					//myMatRef->Load();

					//myDecalGO = myGameObject->GetScene()->AddGameObject<GameObject>();
					//myDecalGO->SetName("MortarDecal");
					//myDecalGO->GetTransform().SetScale(Vec3f(350.f, 350.f, 350.f));
					//auto decal = myDecalGO->AddComponent<Engine::DecalComponent>();
					//decal->GetMaterialInstance().Init(myMatRef);
					//myDecalGO->GetTransform().SetPosition(hit.Position);

					//Vec3f dir = hit.Normal;
					//dir.Normalize();

					//myDecalGO->GetTransform().LookAt(hit.Position + dir * 30.f);
				}
			}
		}

	}

	auto obj = myGameObject->GetScene()->AddGameObject<GameObject>();
	obj->GetTransform().SetParent(&GetTransform());
	obj->GetTransform().SetPositionLocal({ 0,0,0 });

	const auto& vfxRef = myGameObject->GetSingletonComponent<EnemyVFXRef>()->GetMotarProjectileTrail();
	myVFX = obj->AddComponent<Engine::VFXComponent>(vfxRef->Get());
	myVFX->Play();
	myVFX->AutoDestroy();

	Engine::GameObjectPrefab& projectile = myGameObject->GetSingletonComponent<AIDirector>()->GetAirTankProjectile()->Get();
	myModel = &projectile.Instantiate(*myGameObject->GetScene());
	//myModel->GetTransform().SetPosition(myGameObject->GetTransform().GetPosition());
	myModel->GetTransform().SetParent(&myGameObject->GetTransform());
	myModel->GetTransform().SetPositionLocal(Vec3f(0.f, 0.f, 0.f));

	myProjectileHealth = myGameObject->AddComponent<ProjectileHealth>();
	myProjectileHealth->Init(100000000.f, myExplosionRadius, 10.f);

	auto rigidBody = myGameObject->AddComponent<Engine::RigidBodyComponent>(eRigidBodyType::KINEMATIC);
	rigidBody->Attach(Shape::Sphere(myRadius, ePhysicsMaterial::Default, true));
	myGameObject->SetLayers(eLayer::ENEMY_DAMAGEABLE, eLayer::NONE, eLayer::NONE);
	/*rigidBody->ObserveCollision([this](GameObject* obj)
		{
			OnCollision();
		});*/

	myStartPosition = myGameObject->GetTransform().GetPosition();


	RayCastHit hit;
	if (myGameObject->GetScene()->RayCast(
		rayPos,
		Vec3f(0, -1, 0),
		10000.f,
		eLayer::NAVMESH | eLayer::DEFAULT | eLayer::GROUND,
		hit))
	{

		if (hit.GameObject != nullptr)
		{
			myTargetPosition = hit.Position;

			/*GDebugDrawer->DrawCircle3D(
				DebugDrawFlags::Always,
				hit.Position,
				20.f,
				3.f,
				{ 1.f, 1.f, 0.f, 1.f });*/
		}
	}
	//GDebugDrawer->DrawCircle3D(
	//	DebugDrawFlags::Always,
	//	rayPos,
	//	20.f,
	//	3.f,
	//	{ 1.f, 0.f, 1.f, 1.f });
}

void MortarProjectile::Execute(Engine::eEngineOrder aOrder)

{
	Projectile::Execute(aOrder);
	Update();
}

void MortarProjectile::Update()
{
	DestroyUglyFix(); // UGLY


	if (!CheckLifeTime())
	{
		DestroyAll();
		//myDecalGO->Destroy();

		if (!myVFXObj.expired())
		{
			myVFX->Stop();
		}
	}

	/*if (myModel)
	{
		myModel->GetTransform().SetPosition(myGameObject->GetTransform().GetPosition());
	}*/

	CreateSphereCast();

	if (CurveMovement())
	{
		//DropMovement();
	}

	//RotateTowardsPosition();
	//MoveInForward();


	GDebugDrawer->DrawSphere3D(
		DebugDrawFlags::AI,
		myGameObject->GetTransform().GetPosition(),
		5.f,
		0.f,
		{ 1.f, 0.f, 1.f, 1.f });
}

void MortarProjectile::Reflect(Engine::Reflector& aReflector)
{
	Projectile::Reflect(aReflector);
}

void MortarProjectile::Render()
{
	Projectile::Render();
}

//void MortarProjectile::OnCollision()
//{
//	//myProjectileHealth->GetHealthComponent()->ApplyDamage(10000000000.f);
//	LOG_INFO(LogType::Game) << "BurstProjectile.cpp";
//}

void MortarProjectile::Explosion(bool aSouldDealDamage)
{
	std::vector<OverlapHit> hits;
	myGameObject->GetScene()->SphereCastAll(myGameObject->GetTransform().GetPosition(), myExplosionRadius, eLayer::PLAYER_DAMAGEABLE, hits);
	for (size_t i = 0; i < hits.size(); i++)
	{
		if (!hits[i].GameObject)
			continue;

		if (hits[i].GameObject->GetTag() == eTag::ENEMYPROJECTILE)
			continue;

		if (myShooterUUID == hits[i].GameObject->GetUUID())
			continue;

		if (hits[i].GameObject->GetLayer() & eLayer::DAMAGEABLE)
		{
			auto health = hits[i].GameObject->GetComponent<HealthComponent>();

			if (health)
			{
				if (aSouldDealDamage)
					health->ApplyDamage(myDamage);
			}
		}
		GDebugDrawer->DrawSphere3D(
			DebugDrawFlags::AI,
			myGameObject->GetTransform().GetPosition(),
			myExplosionRadius,
			0.5f,
			{ 1.f, 0.f, 0.f, 1.f });
	}
}

void MortarProjectile::CreateSphereCast()
{
	std::vector<OverlapHit> hits;
	myGameObject->GetScene()->SphereCastAll(myGameObject->GetTransform().GetPosition(), myRadius, eLayer::MASK_BURSTPROJECTILE | eLayer::PLAYER_SHIELD, hits);
	for (size_t i = 0; i < hits.size(); i++)
	{
		if (!hits[i].GameObject)
			continue;

		if (hits[i].GameObject->GetTag() == eTag::ENEMYPROJECTILE)
			continue;

		if (hits[i].GameObject->GetLayer() & eLayer::PLAYER_SHIELD)
		{
			Explosion(false);
			if (!myVFXObj.expired())
			{
				myVFX->Stop();
			}
			DestroyAll();

			auto explosionObj = myGameObject->GetScene()->AddGameObject<GameObject>();
			explosionObj->GetTransform().SetPosition(myGameObject->GetTransform().GetPosition());
			explosionObj->AddComponent<GeneralExplosion>(myGameObject->GetSingletonComponent<EnemyVFXRef>()->GetAirTankProjectileExplode()->Get());
			continue;
		}

		if (myShooterUUID == hits[i].GameObject->GetUUID())
			continue;

		Explosion();

		if (!myVFXObj.expired())
		{
			myVFX->Stop();
		}
		DestroyAll();
		//myDecalGO->Destroy();

		auto explosionObj = myGameObject->GetScene()->AddGameObject<GameObject>();
		explosionObj->GetTransform().SetPosition(myGameObject->GetTransform().GetPosition());
		explosionObj->AddComponent<GeneralExplosion>(myGameObject->GetSingletonComponent<EnemyVFXRef>()->GetAirTankProjectileExplode()->Get());
	}
}

void MortarProjectile::RotateTowardsPosition()
{
	auto ownerPos = myGameObject->GetTransform().GetPosition();

	if (myTick)
	{
		myPeekTimer += Time::DeltaTime;
		if (myPeekTimer > myPeekReached)
		{
			myTargetPosition = myGameObject->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos();
			myTick = false;
		}
	}

	myGameObject->GetTransform().LookAt(myTargetPosition);

	const auto& forward = myGameObject->GetTransform().Forward().GetNormalized();
	GDebugDrawer->DrawLine3D(
		DebugDrawFlags::AI,
		ownerPos,
		ownerPos + forward * 300.f,
		0.0f,
		{ 0.f, 0.f, 1.f, 1.f });
}

bool MortarProjectile::CurveMovement()
{
	if (myFlyProgress < myFlyDuration)
	{
		myFlyProgress += Time::DeltaTime;
		if (myFlyProgress >= myFlyDuration)
		{
			myFlyProgress = myFlyDuration;
		}

		//GDebugDrawer->DrawCircle3D(DebugDrawFlags::Always, myStartPosition, 20.f);
		//GDebugDrawer->DrawCircle3D(DebugDrawFlags::Always, myTargetPosition, 20.f);

		float ratio = myFlyProgress / myFlyDuration;
		Vec3f newPos = Math::Lerp(myStartPosition, myTargetPosition, ratio);
		newPos.y = newPos.y + sinf(ratio * CU::PI) * 2000.f;
		LOG_INFO(LogType::Jesper) << ratio;

		myGameObject->GetTransform().SetPosition(newPos);

		if (myindicator)
		{
			auto& children = myindicator->GetTransform().GetChildren();
			for (int i = 0; i < static_cast<int>(children.size()); i++)
			{
				children[i]->SetScale(ratio);
			}
		}

		return false;
	}

	return true;
}

void MortarProjectile::DropMovement()
{
	auto moveDir = Vec3f(0.f, -1.f, 0.f);
	moveDir.Normalize();

	const float speed = 1000.f;
	const auto movement = moveDir * speed * Time::DeltaTime;
	myGameObject->GetTransform().Move(movement);
}

void MortarProjectile::DestroyAll()
{
	myModel->Destroy();
	if (myindicator)
		myindicator->Destroy();

	myGameObject->Destroy();
}

void MortarProjectile::DestroyUglyFix()
{
	float distance = (myTargetPosition - myGameObject->GetTransform().GetPosition()).Length();

	if (distance < 10.f)
	{
		DestroyAll();
	}
}

