#include "pch.h"
#include "MultiRigidStatic.h"

#include "Engine/Engine.h"
#include "../DebugManager/DebugDrawer.h"
#include "Engine/Shortcuts.h"
#include "Engine/GameObject/GameObject.h"
#include "Physics/RigidStatic.h"
#include "Physics/Shape.h"

Engine::MultiRigidStatic::MultiRigidStatic(GameObject* aGameObject)
	: Component(aGameObject)
{

}

Engine::MultiRigidStatic::~MultiRigidStatic()
{
	for (int i = myRigidStatics.size() -1; i >= 0; i--)
	{
		delete myRigidStatics[i];
		myRigidStatics[i] = nullptr;
	}

	Component::~Component();
}

void Engine::MultiRigidStatic::Execute(eEngineOrder aOrder)
{
	ZoneScopedN("MultiRigidStatic::Execute");

	for (auto rigidStatic : myRigidStatics)
	{
		auto rs = rigidStatic->GetActor();
		if (!rs)
			return;

		physx::PxTransform pxTarget = rs->getGlobalPose();

		physx::PxShape** shapes = DBG_NEW physx::PxShape * [1];
		int size = rs->getShapes(shapes, 1, 0);
		if (size > 0)
		{
			CU::Vector4f color = CU::Vector4f(.8f, .8f, .8f, 1);

			auto s = shapes[0];
			auto geoType = s->getGeometryType();
			switch (geoType)
			{
			case physx::PxGeometryType::eSPHERE:
			{
				CU::Vector3f pos{ pxTarget.p.x, pxTarget.p.y, pxTarget.p.z };
				physx::PxSphereGeometry geometry;
				bool isValid = s->getSphereGeometry(geometry);
				if (isValid)
				{
					GDebugDrawer->DrawSphere3D(DebugDrawFlags::Physics, pos, geometry.radius, 0, color);
				}
			}
			break;
			case physx::PxGeometryType::ePLANE: break;
			case physx::PxGeometryType::eCAPSULE:
				
				break;
			case physx::PxGeometryType::eBOX:
			{
				CU::Vector3f pos{ pxTarget.p.x, pxTarget.p.y, pxTarget.p.z };
				CU::Quaternion rot{ pxTarget.q.w, { pxTarget.q.x, pxTarget.q.y, pxTarget.q.z } };
				physx::PxBoxGeometry box;
				bool isValid = s->getBoxGeometry(box);
				if (isValid)
				{
					CU::Vector3f halfWidth{ box.halfExtents.x, box.halfExtents.y, box.halfExtents.z };

					GDebugDrawer->DrawCube3D(DebugDrawFlags::Physics, pos, rot.EulerAngles(), halfWidth, 0, color);
				}
			}
			break;
			case physx::PxGeometryType::eCONVEXMESH: break;
			case physx::PxGeometryType::eTRIANGLEMESH: break;
			case physx::PxGeometryType::eHEIGHTFIELD: break;
			case physx::PxGeometryType::eGEOMETRY_COUNT: break;
			case physx::PxGeometryType::eINVALID: break;
			default:;
			}
		}
	}
}

void Engine::MultiRigidStatic::AddRigidStatic(RigidStatic* aStatic, const CU::Vector3f& aOffset, const CU::Quaternion& aRotation)
{
	myGameObject->GetScene()->AddActor(aStatic);

	auto shapes = aStatic->GetShapes();
	//if (shape->getSimulationFilterData().word0 == eLayer::NONE && shape->getSimulationFilterData().word1 == eLayer::NONE && shape->getSimulationFilterData().word2 == eLayer::NONE)
	//	Shape::SetupFiltering(shape, myGameObject->GetLayer(), eLayer::ALL, eLayer::NONE);

	physx::PxTransform pxTarget;
	auto pos = myGameObject->GetTransform().GetPosition() + aOffset;
	pxTarget.p = { pos.x, pos.y, pos.z };
	pxTarget.q = physx::PxQuat(aRotation.myVector.x, aRotation.myVector.y, aRotation.myVector.z, aRotation.myW);
	aStatic->GetActor()->setGlobalPose(pxTarget);

	aStatic->ConnectUUID(myGameObject->GetUUID());
	myRigidStatics.emplace_back(aStatic);
}
