#include "pch.h"
#include "HealthComponentProxy.h"
#include "Engine/GameObject/GameObject.h"

FB::HealthComponentProxy::HealthComponentProxy(GameObject* aGameObject)
	: HealthComponent(aGameObject)
{
}

void FB::HealthComponentProxy::Start()
{
	if (!myParentGameObject)
	{
		LOG_ERROR(LogType::Game) << myGameObject->GetName() << " is missing parent gameobject ref";
		myGameObject->Destroy();
		return;
	}

	myParentHealthComponent = myParentGameObject.Get()->GetComponent<HealthComponent>();

	// to avoid this child from dying
	const float huugeHealth = 100000000.f;
	SetMaxHealth(huugeHealth);
}

void FB::HealthComponentProxy::Execute(Engine::eEngineOrder aOrder)
{
	// do not call parent execute
	// to avoid this child from dying
	const float huugeHealth = 100000000.f;
	SetMaxHealth(huugeHealth);
}

void FB::HealthComponentProxy::ApplyDamage(
	float aAmount,
	bool aIgnoreShield,
	const CU::Vector3f* anImpactPosition,
	eDamageType aType)
{
	myParentHealthComponent->ApplyDamage(aAmount, aIgnoreShield, anImpactPosition, aType);
}

void FB::HealthComponentProxy::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myParentGameObject, "Parent GameObject");
}
