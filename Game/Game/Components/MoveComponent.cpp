#include "pch.h"
#include "MoveComponent.h"
#include "Engine/Reflection/Reflector.h"
#include "Engine/Scene/SceneManager.h"
#include <AI/PollingStation/PollingStationComponent.h>
#include "Game/Components/HealthComponent.h"
#include "Engine/ResourceManagement/Resources/ModelResource.h"

MoveComponent::MoveComponent(GameObject* aGameObject, const float& aDamage, const float& aSpeed, const float& aDelay, const float& aLifeTime, const float& aAcceleration, const float& aRadius)
	: Component(aGameObject)
{
	myDamage = aDamage;
	mySpeed = aSpeed;
	myDelay = aDelay;
	myLifeTime = aLifeTime;
	myRadius = aRadius;
	myAcceleration = aAcceleration;
}

void MoveComponent::Start()
{
	myMovePart = { 0,0,0 };
	myTargetPos = myGameObject->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos();
	if (myGameObject->GetComponent<HealthComponent>())
	{
		myGameObject->GetComponent<HealthComponent>()->ApplyDamage(myDamage);
	}

}

void MoveComponent::Init(const Vec3f& aPos)
{
	myTargetPos = aPos;
	myTargetPos = { myTargetPos.x, myTargetPos.y + 10.0f, myTargetPos.z };
}

void MoveComponent::Execute(Engine::eEngineOrder aOrder)
{
	Vec4f stressColor;
	Vec4f green = { 0,1,0,1 };
	Vec4f red = { 1,0,0,1 };
	GDebugDrawer->DrawSphere3D(DebugDrawFlags::Gameplay, myTargetPos, 100.0f, 0, Common::Vector4f(1, 1, 0.01f, 1));
	myDelayTimer += Time::DeltaTime;
	if (myDelayTimer > myDelay)
	{
		if (myAcceleration >= mySmoothing)
		{
			mySmoothing += Time::DeltaTime;
			if (mySmoothing / myAcceleration > 1.0f)
			{
				mySmoothing = myAcceleration;
			}
			stressColor = Common::Lerp(green, red, mySmoothing / myAcceleration);

		}
		GDebugDrawer->DrawLine3D(DebugDrawFlags::Gameplay, myTargetPos, myGameObject->GetTransform().GetPosition(), 0, stressColor);
		GDebugDrawer->DrawSphere3D(DebugDrawFlags::Gameplay, myTargetPos, myRadius*0.5f, 0, Common::Vector4f(1, 0.11f, 0.01f, 1));
		myProgress += ((Time::DeltaTime * mySpeed) * mySmoothing);
		if (myGameObject != nullptr)
		{
			if (!myGameObject->IsDestroyed())
			{
				myMovePart = (myTargetPos - myGameObject->GetTransform().GetPosition()) * myProgress;
				myGameObject->GetTransform().Move(myMovePart);
			}
		}
		if (myProgress > 0.98f)
		{
			myProgress = 0.979f;
		}
	}
	if (myDelayTimer - myDelay > myLifeTime)
		Destroy();
}

void MoveComponent::Reflect(Engine::Reflector& aReflector)
{

}