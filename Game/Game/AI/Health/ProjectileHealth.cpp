#include "pch.h"
#include "ProjectileHealth.h"
#include "AI/BehaviorTree/Base/BT_Blackboard.h"

#include "Components/HealthComponent.h"
#include "Engine/Shortcuts.h"
#include "Engine/GameObject/GameObject.h"

#include "Engine\Reflection\Reflector.h"

ProjectileHealth::ProjectileHealth()
{
}

ProjectileHealth::ProjectileHealth(GameObject* aGameObject) :
	Component(aGameObject)
{
}

ProjectileHealth::~ProjectileHealth()
{
}

void ProjectileHealth::Init(float aMaxHealth, float aExplosionRadius, float aExplosionDamage)
{
	myMaxHealth = aMaxHealth;
	myExplosionRadius = aExplosionRadius;
	myExplosionDamage = aExplosionDamage;
}

void ProjectileHealth::SetDeathFunction(std::function<void()> aFunction)
{
	myFunction = aFunction;
}

void ProjectileHealth::Reflect(Engine::Reflector& aReflector)
{

}

void ProjectileHealth::Start()
{
	myMaxHealth = 1.f;
	myHealthComponent = myGameObject->AddComponent<HealthComponent>(myMaxHealth);

	myHealthComponent->ObserveDamage([&](float aDmg)
		{
			OnDamage(aDmg, Vec3f(0.f, 0.f, 0.f));
		});
	//myHealthComponent->SetDamageListener(std::bind(&ProjectileHealth::OnDamage, this, std::placeholders::_1, std::placeholders::_2));
	myHealthComponent->ObserveDeath([&](float aDmg)
		{
			OnDeath(aDmg);
		});

	//myHealthComponent->SetDeathListener(std::bind(&ProjectileHealth::OnDeath, this, std::placeholders::_1));
}

void ProjectileHealth::Execute(Engine::eEngineOrder aOrder)
{
}

void ProjectileHealth::OnDamage(float aDamage, const Vec3f& aDamageDir)
{
	LOG_INFO(LogType::Jesper) << "Projectile Damaged";
}

void ProjectileHealth::OnDeath(float aDamage)
{
	std::vector<OverlapHit> hits;
	myGameObject->GetScene()->SphereCastAll(myGameObject->GetTransform().GetPosition(), myExplosionRadius, eLayer::PLAYER, hits);
	for (size_t i = 0; i < hits.size(); i++)
	{
		if (!hits[i].GameObject)
			continue;

		auto health = hits[i].GameObject->GetComponent<HealthComponent>();

		if (health)
		{
			//health->ApplyDamage(myExplosionDamage);
		}
	}
	GDebugDrawer->DrawSphere3D(
		DebugDrawFlags::AI,
		myGameObject->GetTransform().GetPosition(),
		myExplosionRadius,
		0.5f,
		{ 1.f, 0.f, 0.f, 1.f });
	myGameObject->Destroy();
	if (myFunction != nullptr)
	{
		myFunction();
	}
}

HealthComponent* ProjectileHealth::GetHealthComponent()
{
	return myHealthComponent;
}

