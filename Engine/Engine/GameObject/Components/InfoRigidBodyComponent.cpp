#include "pch.h"
#include "InfoRigidBodyComponent.h"

#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/ComponentSystem/ComponentFactory.h"
#include <Engine/Reflection/Reflector.h>
#include <Engine/Physics/PhysicsDebugDrawer.h>
#include "Engine/Scene/SceneManager.h"

InfoRigidBodyComponent::InfoRigidBodyComponent(GameObject* aGameObject, eRigidBodyType aType)
	: Component(aGameObject)
{
	myType = aType;
}

InfoRigidBodyComponent::~InfoRigidBodyComponent()
{
}
void InfoRigidBodyComponent::Reflect(Engine::Reflector& aReflector)
{
	//Must be in all components
	Component::Reflect(aReflector);

	aReflector.Reflect(myLayer, "Collision layers");

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
	auto color = myType == eRigidBodyType::DEFAULT ? P_RIGID_COLOR : (myType == eRigidBodyType::KINEMATIC ? P_KINEMATIC_COLOR : P_TRIGGER_COLOR);

	for (auto& s : myShapes)
	{
		DebugDrawShape(s, myGameObject->GetTransform().GetPosition(), myGameObject->GetTransform().GetRotation(), color);
		//GDebugDrawer->DrawCube3D(DebugDrawFlags::Physics, myGameObject->GetTransform().GetPosition(), s.GetBoxHalfSize());
	}
}

void InfoRigidBodyComponent::Start()
{
	if (IsActive())
	{
		auto* rb = myGameObject->AddComponent<Engine::RigidBodyComponent>(myType);
		//auto rb =  DBG_NEW Engine::RigidBodyComponent(myGameObject);

		rb->SetLocalRotation(myLocalRotation);
		rb->GetActor()->setGlobalPose(myPxTransform);

		rb->SetOffset(myOffset);
		//rb->SetMass(myMass);

		for (auto& s : myShapes)
		{
			rb->Attach(Shape::Copy(s));
		}

		//myGameObject->RemoveComponent(this);
	}
}

void InfoRigidBodyComponent::Attach(Shape aShape)
{
	myShapes.emplace_back(aShape);
}

void InfoRigidBodyComponent::SetMass(float aMass)
{
	myMass = aMass;
}

void InfoRigidBodyComponent::SetOffset(const CU::Vector3f& aOffset)
{
	assert(false && "This offset does not work!");
	myOffset = aOffset;
}

const CU::Vector3f& InfoRigidBodyComponent::GetOffset() const
{
	return myOffset;
}

void InfoRigidBodyComponent::SetLocalRotation(const CU::Quaternion& aRotation)
{
	myLocalRotation = aRotation;
}

const CU::Quaternion& InfoRigidBodyComponent::GetLocalRotation() const
{
	return myLocalRotation;
}

void InfoRigidBodyComponent::SetTransform(const CU::Quaternion& aQuaternion, const CU::Vector3f& aPosition)
{
	myPxTransform.p = { aPosition.x, aPosition.y, aPosition.z };
	myPxTransform.q = physx::PxQuat(aQuaternion.myVector.x, aQuaternion.myVector.y, aQuaternion.myVector.z, aQuaternion.myW);
}
void InfoRigidBodyComponent::DebugDrawShape(const Shape& aShape, const CU::Vector3f& aPosition, const CU::Quaternion& aRotation, const CU::Vector4f& aColor)
{
	auto& debugDrawer = myGameObject->GetScene()->GetRendererScene().GetDebugDrawer();

	if (aShape.IsValid())
	{
		auto pos = aPosition + aShape.GetLocalPosition() * aRotation;
		auto halfsize = aShape.GetBoxHalfSize();
		switch (aShape.GetType())
		{
		case Shape::eGeometry::eBOX:
		{
			debugDrawer.DrawCube3D(
				DebugDrawFlags::Physics,
				pos,
				aRotation.EulerAngles(),
				aShape.GetBoxHalfSize(),
				0,
				aColor);
		}
		break;
		case Shape::eGeometry::eSPHERE:
		{
			debugDrawer.DrawSphere3D(DebugDrawFlags::Physics, pos, aShape.GetRadius(), 0, aColor);
		}
		break;
		case Shape::eGeometry::eCAPSULE:
		{
			const auto radius = aShape.GetRadius();
			const auto height = aShape.GetHeight();

			debugDrawer.DrawLine3D(
				DebugDrawFlags::Physics,
				pos + CU::Vector3f{ 0, height * .5f, 0 },
				pos - CU::Vector3f{ 0, height * .5f, 0 },
				0,
				aColor);

			debugDrawer.DrawSphere3D(
				DebugDrawFlags::Physics,
				pos + CU::Vector3f{ 0, height * .5f, 0 },
				radius,
				0,
				aColor);

			debugDrawer.DrawSphere3D(
				DebugDrawFlags::Physics,
				pos - CU::Vector3f{ 0, height * .5f, 0 },
				radius,
				0,
				aColor);
		}
		break;
		case Shape::eGeometry::eCONVEX:
		{
			auto vertices = aShape.GetConvexVertices();
			for (int i = 0; i < vertices.size(); i++)
			{
				auto posA = vertices[((i - 1)) % vertices.size()];
				auto posB = vertices[(i) % vertices.size()];
				debugDrawer.DrawLine3D(
					DebugDrawFlags::Physics,
					pos + posA,
					pos + posB,
					0,
					aColor);
			}
		}
		break;
		}


		/*
		Shape::eGeometry geometry = aShape.

		switch (geoType)
		{
		case physx::PxGeometryType::eSPHERE:
		{
			CU::Vector3f pos{ pxTarget.p.x, pxTarget.p.y, pxTarget.p.z };
			physx::PxSphereGeometry geometry;
			bool isValid = s->getSphereGeometry(geometry);
			if (isValid)
			{
				GetEngine().GetDebugDrawer().DrawSphere3D(pos, geometry.radius, 0, color);
			}
		}
		break;
		case physx::PxGeometryType::ePLANE: break;
		case physx::PxGeometryType::eCAPSULE: break;
		case physx::PxGeometryType::eBOX:
		{
			CU::Vector3f pos{ pxTarget.p.x, pxTarget.p.y, pxTarget.p.z };
			CU::Quaternion rot{ pxTarget.q.w, { pxTarget.q.x, pxTarget.q.y, pxTarget.q.z } };
			physx::PxBoxGeometry box;
			bool isValid = s->getBoxGeometry(box);
			if (isValid)
			{
				CU::Vector3f halfWidth{ box.halfExtents.x, box.halfExtents.y, box.halfExtents.z };

				Engine::GetInstance().GetDebugDrawer().DrawCube3D(pos, rot.EulerAngles(), halfWidth, 0, color);
			}
		}
		break;
		case physx::PxGeometryType::eCONVEXMESH: break;
		case physx::PxGeometryType::eTRIANGLEMESH: break;
		case physx::PxGeometryType::eHEIGHTFIELD: break;
		case physx::PxGeometryType::eGEOMETRY_COUNT: break;
		case physx::PxGeometryType::eINVALID: break;
		default:;
		}*/
	}
}