#include "pch.h"
#include "RespawnComponent.h"
#include "Engine/Reflection/Reflector.h"
#include "Engine/GameObject/GameObject.h"

#include "Game/Components/HealthComponent.h"
#include "Engine\GameObject\Components/RigidBodyComponent.h"
#include "Engine\GameObject\Components\Collider.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/GameObject/Components/VFXComponent.h"

#include <vector>

RespawnComponent::RespawnComponent(GameObject* aGameObject)
{
	myGameObject = aGameObject;
}

RespawnComponent::~RespawnComponent()
{
}

void RespawnComponent::Start()
{
	//auto rigBod = myGameObject->GetComponent<Engine::RigidBodyComponent>();
	//rigBod->ObserveTriggerEnter([&](GameObject* aGameObject) { OnTriggerEnter(aGameObject); });
	//rigBod->ObserveTriggerStay([&](GameObject* aGameObject) { OnTriggerStay(aGameObject); });

	myPlayer = Main::GetPlayer();

	myHealthComponent = myPlayer->GetComponent<HealthComponent>();
	if (!myHealthComponent)
		myHealthComponent = myPlayer->AddComponent<HealthComponent>();

	myLastPosition = myPlayer->GetTransform().GetPosition();
}

void RespawnComponent::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myDamagePercent, "Damage Percent");
	aReflector.Reflect(myTime, "Ground Check Interval");
}

void RespawnComponent::Execute(Engine::eEngineOrder aOrder)
{
	if (!myPlayer)
		return;

	myTimer += Time::DeltaTime;
	if (myTimer > myTime)
	{
		myTimer = 0.0f;

		RayCastHit hit;
		if (myGameObject->GetScene()->RayCast(myPlayer->GetTransform().GetPosition() + Vec3f(0.0f, 100.0f, 0.0f), Vec3f(0.0f, -1.0f, 0.0f), 800.0f, eLayer::NAVMESH, hit))
		{
			myLastPosition = hit.Position;
			myLastPosition.y += 100.0f;
		}
	}

	if (myPlayer->GetTransform().GetPosition().y < GetTransform().GetPosition().y)
	{
		myShouldTeleportPlayer = false;

		myPlayer->GetTransform().ResetMovement();
		myPlayer->GetTransform().SetPosition(myLastPosition);

		myHealthComponent->ApplyDamage(myHealthComponent->GetMaxHealth() * myDamagePercent);

		LOG_INFO(LogType::Game) << "Player fell off the map, set its positon to " << myLastPosition.x << ", " << myLastPosition.y << ", " << myLastPosition.z;
	}


#ifndef _RETAIL
	Vec3f drawPos = myGameObject->GetScene()->GetMainCameraGameObject()->GetTransform().GetPosition();
	drawPos.y = GetTransform().GetPosition().y;

	Vec3f offset1(500.0f, 0.0f, 0.0f);
	Vec3f offset2(0.0f, 0.0f, 500.0f);

	GDebugDrawer->DrawLine3D(DebugDrawFlags::Gameplay, drawPos + offset1, drawPos - offset1, 0.0f, Vec4f(1.0f, 0.0f, 0.0f, 1.0f));
	GDebugDrawer->DrawLine3D(DebugDrawFlags::Gameplay, drawPos + offset2, drawPos - offset2, 0.0f, Vec4f(1.0f, 0.0f, 0.0f, 1.0f));
#endif // !_RETAIL
}