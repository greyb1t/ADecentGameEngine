#include "pch.h"
#include "RigidStaticComponent.h"

#include "RigidBodyComponent.h"
#include "../GameObject.h"
#include "../../Engine.h"
#include "../../DebugManager/DebugDrawer.h"
#include "Physics/Shape.h"
#include "Engine/Shortcuts.h"
#include "Engine/Physics/PhysicsDebugDrawer.h"

Engine::RigidStaticComponent::RigidStaticComponent(GameObject* aGameObject)
	: Component(aGameObject), RigidStatic()
{
	myScene = myGameObject->GetScene();

	auto pos = myGameObject->GetTransform().GetPosition();
	auto rot = myGameObject->GetTransform().GetRotation();
	if (!RigidStatic::Init(pos, rot, myGameObject->GetLayer(), myGameObject->GetPhysicalLayerMask(), myGameObject->GetDetectionLayerMask()))
	{
		assert(false && "PhysX FAILED TO INIT RIGIDSTATIC");
		return;
	}

	myGameObject->GetScene()->AddActor(this);
	ConnectUUID(myGameObject->GetUUID());
}

Engine::RigidStaticComponent::~RigidStaticComponent()
{
	RigidStatic::~RigidStatic();
}

void Engine::RigidStaticComponent::Start()
{
}

void Engine::RigidStaticComponent::Execute(eEngineOrder aOrder)
{
	const auto layer = myGameObject->GetLayer();
	const auto physicalLayerMask = myGameObject->GetPhysicalLayerMask();
	const auto detectionLayerMask = myGameObject->GetDetectionLayerMask();
	if (myFilterGroup != layer
		|| myPhysicalFilterTargets != physicalLayerMask
		|| myDetectionFilterTargets != detectionLayerMask)
	{
		SetupFiltering(layer, physicalLayerMask, detectionLayerMask);
	}

	DebugDraw();
}

Component* Engine::RigidStaticComponent::Clone() const
{
	return nullptr;
}

void Engine::RigidStaticComponent::SetOnCollisionListener(const std::function<void(GameObject*)>& aFunction)
{
	myOnCollisionObserver = aFunction;
}

void Engine::RigidStaticComponent::SetOnEnterCollisionListener(const std::function<void(GameObject*)>& aFunction)
{
	myOnCollisionEnterObserver = aFunction;
}

void Engine::RigidStaticComponent::SetOnExitCollisionListener(const std::function<void(GameObject*)>& aFunction)
{
	myOnCollisionExitObserver = aFunction;
}

void Engine::RigidStaticComponent::DebugDraw()
{
	if (!GetEngine().GetDebugDrawerSettings().IsDebugFlagActive(DebugDrawFlags::Physics))
	{
		return;
	}

	if (!myRigidStatic)
		return;

	auto color = P_STATIC_COLOR;
	for (auto& s : myShapes)
	{
		PhysicsDebugDrawer::DebugDrawShape(s, myGameObject->GetTransform().GetPosition(), myGameObject->GetTransform().GetRotation(), color);
	}
	return;

	physx::PxTransform pxTarget = myRigidStatic->getGlobalPose();

	physx::PxShape** shapes = DBG_NEW physx::PxShape * [1];
	int size = myRigidStatic->getShapes(shapes, 1, 0);
	if (size > 0)
	{
		CU::Vector4f color = CU::Vector4f(.8f, .8f, .8f, 1);

		if (IsCollisionListener())
		{
			auto& collisions = myGameObject->GetScene()->GetCollisionRegistry().GetData(myUUID);

			color = CU::Vector4f(.4f, .9f, .4f, 1);

			if (!collisions.empty())
			{
				color = CU::Vector4f(.9f, .3f, .3f, 1);
			}

		}

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
