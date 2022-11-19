#include "pch.h"
#include "Dummy.h"

#include "Components/HealthComponent.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/RigidBodyComponent.h"
#include "Engine/Reflection/Reflector.h"

Dummy::Dummy(GameObject* aGameObject)
	: Component(aGameObject)
{

}

void Dummy::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myConstrainRotation, "Constrain Rotation");
}

void Dummy::Start()
{
	myGameObject->SetLayers(eLayer::ENEMY | eLayer::DAMAGEABLE, eLayer::GROUND | eLayer::DEFAULT, eLayer::NONE);

	myRb = myGameObject->GetComponent<Engine::RigidBodyComponent>();
	if (myConstrainRotation && myRb)
	{
		myRb->SetRotationConstraints(true, true, true);
	}

	myHealth = myGameObject->GetComponent<HealthComponent>();
	if (!myHealth)
	{
		myHealth = myGameObject->AddComponent<HealthComponent>();
	}
	myHealth->SetMaxHealth(1000000);
	myHealth->ObserveDamage([&](float dmg)
	{
		OnDamage(dmg);
	});
	myHealth->ObserveDeath([&](float dmg)
	{
		OnDeath(dmg);
	});
}

void Dummy::Execute(Engine::eEngineOrder aOrder)
{
}

void Dummy::OnDamage(float aDmg)
{
	LOG_INFO(LogType::Viktor) << "Dummy damaged: " << aDmg << " - HP:" << myHealth->GetHealth() << "/" << myHealth->GetMaxHealth();

	if (myRb)
	{
		myRb->SetVelocity({ 0, CU::Min(aDmg * 25.f, 250.f) , 0 });
	}
}

void Dummy::OnDeath(float aDmg)
{
	LOG_INFO(LogType::Viktor) << "Dummy killed: " << aDmg << " - HP:" << myHealth->GetHealth() << "/" << myHealth->GetMaxHealth();
	myGameObject->Destroy();
}
