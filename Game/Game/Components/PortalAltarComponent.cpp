#include "pch.h"
#include "PortalAltarComponent.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/Reflection/Reflector.h"
#include <Engine/GameObject/Components/TriggerComponent.h>
#include <Engine/GameObject/Components/RigidBodyComponent.h>
#include <Engine/GameObject/Components/ModelComponent.h>
#include <Components/InteractableManager.h>

PortalAltarComponent::~PortalAltarComponent()
{
	//InteractableManager::Instance().Unregister(this);
}

void PortalAltarComponent::Start()
{
	ObserveInteract([&]
		{
			myTimerStarted = true;
			myEvent = GeneralEvent::PortalSpawn;
			myGameObject->GetComponent<Interactable>()->SetCanInteract(false);
		});

	//InteractableManager::Instance().Register(this);
	myEvent = GeneralEvent::PortalTimerStart;
}
void PortalAltarComponent::Execute(Engine::eEngineOrder aOrder)
{
	if (myTimerStarted)
	{
		myTimer += Time::DeltaTime;
		if (myTimer > myPortalSpawnTime)
		{
			SpawnPortal();
			//myGameObject->GetTransform().SetScale(2.f);
		}
	}
}
void PortalAltarComponent::Reflect(Engine::Reflector& aReflector)
{
	//aReflector.Reflect(myGoldAmount, "Gold Amount");
	Interactable::Reflect(aReflector);
}
void PortalAltarComponent::SpawnPortal()
{
	myGameObject->GetTransform().GetParent()->GetChildren()[1]->GetGameObject()->GetComponent<Engine::ModelComponent>()->SetActive(true);
	myEvent = GeneralEvent::PortalSpawn;
}
void PortalAltarComponent::SetEvent(GeneralEvent aEvent)
{
	myEvent = aEvent;
}