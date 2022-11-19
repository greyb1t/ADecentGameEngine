#include "pch.h"
#include <Engine/GameObject/Components/ScriptsComponent.h>
#include "TriggerComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/Reflection/Reflector.h"
#include <Engine/GameObject/Components/RigidBodyComponent.h>
#include <Engine/TriggerEventManager/TriggerEventManager.h>
#include <Engine/GeneralEventManager/GeneralEventManager.h>
//#include "../../../../Game/Game/Components/PortalComponent.h"
#include "Engine/GeneralEventManager/GeneralEventManager.h"
//#include "../../../../Game/Game/Components/PortalAltarComponent.h"

Engine::TriggerComponent::TriggerComponent(GameObject* aGameObject)
	: Component(aGameObject)
{
}
Engine::TriggerComponent::TriggerComponent(GameObject* aGameObject, Vec3f aSize, std::string aLayer, std::string aEvent)
	: Component(aGameObject)
{
	myEvent = aEvent;
	myLayer = aLayer;
	mySize = aSize;
}

void Engine::TriggerComponent::Execute(eEngineOrder aOrder)
{
	triggerCoooldownTimer += GetEngine().GetTimer().GetDeltaTime();
}

void Engine::TriggerComponent::Render()
{
	Vec3f myHalfSize;
	myHalfSize.x = mySize.x * 0.5f;
	myHalfSize.y = mySize.y * 0.5f;
	myHalfSize.z = mySize.z * 0.5f;
	GDebugDrawer->DrawCube3D(
		DebugDrawFlags::Always,
		myGameObject->GetTransform().GetPosition(),
		myGameObject->GetTransform().GetRotation().EulerAngles(),
		myHalfSize);
}

void Engine::TriggerComponent::Reflect(Reflector& aReflector)
{
	//Must be in all components
	Component::Reflect(aReflector);

	aReflector.Reflect(myIsInteractor, "Is Trigger Interactor");
	aReflector.Reflect(myEvent, "Event");
	aReflector.Reflect(myLayer, "Layer");
	auto flag = aReflector.Reflect(mySize, "Size");
	if ((flag & ReflectorResult_Changed) || (flag & ReflectorResult_Changing))
	{
		myShape = Shape::Box(mySize);
	}
}
void Engine::TriggerComponent::SetLayer(std::string aLayer)
{
	myLayer = aLayer;
}
void Engine::TriggerComponent::SetSize(Vec3f aSize)
{
	mySize = aSize;
}
void Engine::TriggerComponent::SetEvent(std::string aEvent)
{
	myEvent = aEvent;
}
void Engine::TriggerComponent::Start()
{
	if (IsActive())
	{
		myShape = Shape::Box(mySize);
		myGameObject->SetLayers(eLayer::NONE, eLayer::NONE, eLayer::NONE);

		auto& playerTransform = myGameObject->GetTransform();
		auto* rb = DBG_NEW RigidBodyComponent();
		if (myIsInteractor)
		{
			rb->Destroy();
			rb = myGameObject->AddComponent<RigidBodyComponent>(eRigidBodyType::KINEMATIC);
			myGameObject->SetLayers(eLayer::NONE, eLayer::NONE, eLayer::PLAYER);
		}
		else
		{
			rb->Destroy();
			rb = myGameObject->AddComponent<RigidBodyComponent>(eRigidBodyType::TRIGGER);
		}
		//auto rb =  new Engine::RigidBodyComponent(myGameObject);

		rb->SetLocalRotation(myGameObject->GetTransform().GetRotationLocal());

		myPxTransform.p = { playerTransform.GetPosition().x, playerTransform.GetPosition().y, playerTransform.GetPosition().z };
		myPxTransform.q = physx::PxQuat(playerTransform.GetRotation().myVector.x,
			playerTransform.GetRotation().myVector.y,
			playerTransform.GetRotation().myVector.z,
			playerTransform.GetRotation().myW);
		rb->GetActor()->setGlobalPose(myPxTransform);

		rb->SetOffset(myOffset);
		//rb->SetMass(myMass);

		rb->Attach(Shape::Copy(myShape));

		if (!myIsInteractor)
		{
			rb->ObserveTriggerEnter([&](GameObject* obj)
				{
					// Jonte: Super ful dum timer check pågrund av dubbel OnTriggerEnter
					//if (triggerCoooldownTimer > triggerCoooldown)
					{
						if (obj->GetLayer() & eLayer::PLAYER)
						{
							LOG_INFO(LogType::Markus) << "Calling Event" << myEvent;
							/*GeneralEvent event = myGameObject->GetComponent<PortalAltarComponent>()->GetEvent();
							if (event == GeneralEvent::PortalSpawn)
							{
								LOG_INFO(LogType::Markus) << "Calling Event" << "Portal Spawn";
								GeneralEventManager::GetInstance().SendLetter(event, myGameObject);
								myGameObject->GetComponent<PortalAltarComponent>()->SetEvent(GeneralEvent::PortalTimerStart);
							}
							else if (event == GeneralEvent::PortalTimerStart)
							{
								LOG_INFO(LogType::Markus) << "Calling Event" << "Portal Timer Start";
								GeneralEventManager::GetInstance().SendLetter(event, myGameObject);
								myGameObject->GetComponent<PortalAltarComponent>()->SetEvent(GeneralEvent::PortalSpawn);
							}*/

							//else
							//{
							TriggerEventManager::GetInstance().SendLetter(myEvent, obj);
							//}
							triggerCoooldownTimer = 0;
						}
					}
				});
		}
		//myGameObject->RemoveComponent(this);
	}
}
Layer Engine::TriggerComponent::StringToLayer(std::string aLayer)
{
	if (aLayer == "Player")
	{
		return eLayer::PLAYER;
	}
	else if (aLayer == "Enemy")
	{
		return eLayer::ENEMY;
	}
	else if (aLayer == "Ground")
	{
		return eLayer::GROUND;
	}

	else if (aLayer == "Props")
	{
		return eLayer::PROPS;
	}
	else if (aLayer == "Destructable")
	{
		return eLayer::DESTRUCTABLE;
	}
	else if (aLayer == "Fracture")
	{
		return eLayer::FRACTURE;
	}
	else if (aLayer == "All")
	{
		return eLayer::ALL;
	}
	else
	{
		return eLayer::DEFAULT;
	}
}