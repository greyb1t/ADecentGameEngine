#include "pch.h"
#include "InfoRigidStaticComponent.h"

#include "RigidStaticComponent.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/ComponentSystem/ComponentFactory.h"
#include <Physics/RigidBody.h>
#include <Engine/Reflection/Reflector.h>
#include <Engine/Physics/PhysicsDebugDrawer.h>
#include "Engine/Scene/SceneManager.h"

InfoRigidStaticComponent::InfoRigidStaticComponent(GameObject* aGameObject)
{
	myGameObject = aGameObject;
}

InfoRigidStaticComponent::~InfoRigidStaticComponent()
{
}

void InfoRigidStaticComponent::Start()
{
	if (IsActive())
	{
		//auto* rs = DBG_NEW Engine::RigidStaticComponent(myGameObject);
		const Quatf oldRot = GetTransform().GetRotation();

		//GetTransform().SetPosition({0,0,0});
		if (!myShapes.empty()) {
			if (myShapes[0].GetType() == Shape::eCONVEX
				|| myShapes[0].GetType() == Shape::eBOX
				|| myShapes[0].GetType() == Shape::eCAPSULE) {
				GetTransform().SetRotation(Quatf());
			}
		}
		auto* rs = myGameObject->AddComponent<Engine::RigidStaticComponent>();
		rs->SetTrigger(myIsTrigger);
		//rs->GetActor()->setGlobalPose(myPxTransform);
		rs->SetCollisionListener(true);
		//myGameObject->SetLayer(static_cast<Layer>(GetLayerFromString(myLayer))); // Make layers weird Signed Jesper

		for (auto& s : myShapes)
		{
			rs->Attach(Shape::Copy(s));
		}

		GetTransform().SetRotation(oldRot);
	}

	//myGameObject->RemoveComponent(this);
	// Fix this
	/*
		Om denna rad är aktiv kommer bara (antalet shapes - 1) in i Runtime
	*/

}

void InfoRigidStaticComponent::Attach(Shape aShape)
{
	assert(aShape.IsValid());
	myShapes.emplace_back(aShape);
}

void InfoRigidStaticComponent::Reflect(Engine::Reflector& aReflector)
{
	Component::Reflect(aReflector);

	aReflector.Reflect(myLayer, "Collision layer");

	aReflector.Reflect(myIsTrigger, "Is Trigger");

	aReflector.Reflect(myOffset, "Offset");
	auto flag = aReflector.Reflect(mySize, "Size");
	if ((flag & Engine::ReflectorResult_Changed) || (flag & Engine::ReflectorResult_Changing))
	{
		myShape = Shape::Box(mySize);
		if (myShapes.empty())
		{
			myShapes.push_back(myShape);
		}
		else
		{
			myShapes[0] = myShape;
		}
	}
	//auto color = myType == eRigidBodyType::DEFAULT ? P_RIGID_COLOR : (myType == eRigidBodyType::KINEMATIC ? P_KINEMATIC_COLOR : P_TRIGGER_COLOR);

	for (auto& s : myShapes)
	{
		//GDebugDrawer->DrawCube3D(DebugDrawFlags::Physics, myGameObject->GetTransform().GetPosition(), s.GetBoxHalfSize());
	}
}

void InfoRigidStaticComponent::SetTransform(const CU::Quaternion& anQuaternion, const CU::Vector3f& aPosition)
{
	myQuat = anQuaternion;
	myPos = aPosition;

	//myPxTransform.p = { aPosition.x, aPosition.y, aPosition.z };
	//myPxTransform.q = physx::PxQuat(anQuaternion.myVector.x, anQuaternion.myVector.y, anQuaternion.myVector.z, anQuaternion.myW);
	//GetTransform().SetPosition(aPosition);
	//GetTransform().SetRotation(anQuaternion);
	//myPxTransform.p = { 0,0,0 };
	//myPxTransform.q = physx::PxQuat(0,0,0,1);
}

int InfoRigidStaticComponent::GetLayerFromString(const std::string& aLayerName)
{
	if (myLayer == "Player")
	{
		return 4;
	}
	return -1;
}
