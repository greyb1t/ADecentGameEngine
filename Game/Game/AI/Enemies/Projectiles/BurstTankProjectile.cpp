#include "pch.h"
#include "BurstTankProjectile.h"

#include "AI/Health/ProjectileHealth.h"
#include "Components/HealthComponent.h"
#include "Engine/Shortcuts.h"
#include "Engine/GameObject/Components/RigidBodyComponent.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/VFX/VFXUtilities.h"

BurstTankProjectile::BurstTankProjectile(GameObject* aGameObject) :
	Projectile(aGameObject)
{
}

void BurstTankProjectile::Start()
{
	Projectile::Start();
	myGameObject->SetName("BurstTankProjectile");

	auto obj = myGameObject->GetScene()->AddGameObject<GameObject>();
	obj->GetTransform().SetParent(&GetTransform());
	obj->GetTransform().SetPositionLocal({ 0,0,0 });
	//auto* vfx = obj->AddComponent<Engine::VFXComponent>(VFX::Load("Assets/VFX/RapidProjectile.vfx"));
	//vfx->Play();

	myProjectileHealth = myGameObject->AddComponent<ProjectileHealth>();
	myProjectileHealth->Init(1.f, 1.f, 1.f);

	auto rigidBody = myGameObject->AddComponent<Engine::RigidBodyComponent>(eRigidBodyType::KINEMATIC);
	rigidBody->Attach(Shape::Sphere(30.f, ePhysicsMaterial::Default, true));
	myGameObject->SetLayers(eLayer::ENEMY_DAMAGEABLE, eLayer::NONE, eLayer::PLAYER);
	rigidBody->ObserveCollision([this](GameObject* obj)
		{
			OnCollision();
		});

	


}

void BurstTankProjectile::Execute(Engine::eEngineOrder aOrder)
{
	Projectile::Execute(aOrder);
	Update();
}

void BurstTankProjectile::Update()
{
	if (!CheckLifeTime())
	{
		myGameObject->Destroy();
		Destroy();
	}

	RayCastHit hit;
	if (CheckCollisionAndMove(eLayer::ALL - eLayer::ENEMY, hit, true))
	{
		if (hit.GameObject && hit.GameObject->GetLayer() & /*eLayer::ALL - eLayer::ENEMY*/ eLayer::PLAYER)
		{
			myProjectileHealth->GetHealthComponent()->ApplyDamage(10000000000.f);
		}
	}

	GDebugDrawer->DrawSphere3D(
		DebugDrawFlags::AI,
		myGameObject->GetTransform().GetPosition(),
		5.f,
		0.f,
		{ 1.f, 0.f, 1.f, 1.f });
}

void BurstTankProjectile::Reflect(Engine::Reflector& aReflector)
{
	Projectile::Reflect(aReflector);
}

void BurstTankProjectile::Render()
{
	Projectile::Render();
}

void BurstTankProjectile::OnCollision()
{
	myProjectileHealth->GetHealthComponent()->ApplyDamage(10000000000.f);
	LOG_INFO(LogType::Game) << "BurstProjectile.cpp";
}

