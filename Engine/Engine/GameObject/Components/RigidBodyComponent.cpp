#include "pch.h"
#include "RigidBodyComponent.h"

#include "..\Engine.h"
#include "../GameObject.h"
#include "Engine/Scene/Scene.h"
#include "..\DebugManager/DebugDrawer.h"
#include "Physics/Shape.h"
#include "Engine/Physics/PhysicsDebugDrawer.h"

Engine::RigidBodyComponent::RigidBodyComponent()
{

}

Engine::RigidBodyComponent::RigidBodyComponent(GameObject* aGameObject, eRigidBodyType aType)
	: Component(aGameObject)
	, RigidBody(
		aType,
		myGameObject->GetTransform().GetPosition(),
		myGameObject->GetTransform().GetRotation(),
		myGameObject->GetLayer(),
		myGameObject->GetPhysicalLayerMask(),
		myGameObject->GetDetectionLayerMask())
{
	myScene = myGameObject->GetScene();
	myGameObject->GetScene()->AddActor(this);
	ConnectUUID(myGameObject->GetUUID());
}

Engine::RigidBodyComponent::~RigidBodyComponent()
{
}

Component* Engine::RigidBodyComponent::Clone() const
{
	return nullptr;
}

void Engine::RigidBodyComponent::Start()
{
}

//bool Engine::RigidBodyComponent::Init(physx::PxShape* aShape, eRigidBodyType aRigidBody, bool aIsTrigger)
//{
//	//if (aShape->getSimulationFilterData().word0 == eLayer::NONE && aShape->getSimulationFilterData().word1 == eLayer::NONE && aShape->getSimulationFilterData().word2 == eLayer::NONE)
//	//	Shape::SetupFiltering(aShape, myGameObject->GetLayer(), eLayer::ALL, eLayer::NONE);
//	//
//	//if (!RigidBody::Init(aShape, aRigidBody, aIsTrigger))
//	//{
//	//	return false;
//	//}
//
//	myGameObject->GetScene()->AddActor(*myRigidBody);
//	myScene = myGameObject->GetScene();
//	// TODO: Maybe change from all?
//	ConnectUUID(myGameObject->GetUUID());
//	return true;
//}

void Engine::RigidBodyComponent::Execute(eEngineOrder aOrder)
{
	ZoneScopedN("RigidBodyComponent::Execute");

	switch (aOrder)
	{
	case EARLY_PHYSICS: EarlyPhysics(Time::DeltaTime); return;
	case LATE_PHYSICS:	LatePhysics(Time::DeltaTime); DebugDraw();
	}
}

void Engine::RigidBodyComponent::SetOffset(const CU::Vector3f& aOffset)
{
	myOffset = aOffset;
}

const CU::Vector3f& Engine::RigidBodyComponent::GetOffset() const
{
	return myOffset;
}

void Engine::RigidBodyComponent::SetLocalRotation(const CU::Quaternion& aRotation)
{
	myLocalRotation = aRotation;
}

const CU::Quaternion& Engine::RigidBodyComponent::GetLocalRotation() const
{
	return myLocalRotation;
}

void Engine::RigidBodyComponent::ObserveCollision(const std::function<void(GameObject*)>& aFunction)
{
	if (!myCollisionListener)
	{
		myScene->GetCollisionRegistry().RegisterUUID(myUUID, true);
	}
	myCollisionListener = true;
	myCollisionObserver = aFunction;
}

void Engine::RigidBodyComponent::ObserveTriggerStay(const std::function<void(GameObject*)>& aFunction)
{
	if (!myCollisionListener)
	{
		myScene->GetCollisionRegistry().RegisterUUID(myUUID, true);
	}
	myCollisionListener = true;
	myTriggerStayObserver = aFunction;
}

void Engine::RigidBodyComponent::ObserveTriggerEnter(const std::function<void(GameObject*)>& aFunction)
{
	if (!myCollisionListener)
	{
		myScene->GetCollisionRegistry().RegisterUUID(myUUID, true);
	}
	myCollisionListener = true;
	myTriggerEnterObserver = aFunction;
}

void Engine::RigidBodyComponent::ObserveTriggerExit(const std::function<void(GameObject*)>& aFunction)
{
	if (!myCollisionListener)
	{
		myScene->GetCollisionRegistry().RegisterUUID(myUUID, true);
	}
	myCollisionListener = true;
	myTriggerExitObserver = aFunction;
}

void Engine::RigidBodyComponent::EarlyPhysics(float aDeltaTime)
{
	if (!myRigidBody)
		return;

	const auto layer = myGameObject->GetLayer();
	const auto physicalLayerMask = myGameObject->GetPhysicalLayerMask();
	const auto detectionLayerMask = myGameObject->GetDetectionLayerMask();

	if (myFilterGroup != layer
		|| myPhysicalFilterTargets != physicalLayerMask
		|| myDetectionFilterTargets != detectionLayerMask)
	{
		SetupFiltering(layer, physicalLayerMask, detectionLayerMask);
	}

	physx::PxTransform pxTarget = myRigidBody->getGlobalPose();

	auto& transform = myGameObject->GetTransform();
	if (transform.GetMovement() != CU::Vector3f(0, 0, 0))
	{
		Move(transform.GetMovement());
		SetRotation(transform.GetRotated());
		myGameObject->GetTransform().ResetMovement();
		return;
	}

	// TODO: Maybe add check for if the position & rotation is same as last frame

	CU::Quaternion q = transform.GetRotation() * myLocalRotation;
	auto matrix = q.ToMatrix();

	auto offset = myOffset.z * matrix.GetForward() + myOffset.y * matrix.GetUp() + myOffset.x * matrix.GetRight();
	pxTarget.p = { physx::PxReal(transform.GetPosition().x + offset.x), physx::PxReal(transform.GetPosition().y + offset.y), physx::PxReal(transform.GetPosition().z + offset.z) };


	pxTarget.q = physx::PxQuat(q.myVector.x, q.myVector.y, q.myVector.z, q.myW);
	//pxTarget.q = physx::PxQuat(transform.GetRotation().myVector.x, transform.GetRotation().myVector.y, transform.GetRotation().myVector.z, transform.GetRotation().myW);
	myRigidBody->setGlobalPose(pxTarget);
}

void Engine::RigidBodyComponent::LatePhysics(float aDeltaTime)
{
	if (!myRigidBody)
		return;

	if (myType == eRigidBodyType::DEFAULT)
	{
		auto& transform = myGameObject->GetTransform();

		physx::PxTransform pxTarget = myRigidBody->getGlobalPose();
		transform.SetPosition(CU::Vector3f{ pxTarget.p.x, pxTarget.p.y, pxTarget.p.z } - myOffset);
		transform.SetRotation(CU::Quaternion(pxTarget.q.w, CU::Vector3f(pxTarget.q.x, pxTarget.q.y, pxTarget.q.z)));
	}

	if (myCollisionListener)
	{
		auto& collisions = myScene->GetCollisionRegistry().GetData(GetUUID());
		if (!collisions.empty())
		{
			//std::cout << "_________________________" << std::endl;
			for (auto& c : collisions)
			{
				auto& data = c.a.uuid == myUUID ? c.b : c.a;

				//std::cout << "Collision - UUID( " << data.uuid << " )  State: " << (c.state == eCollisionState::OnEnter ? "OnEnter" : c.state == eCollisionState::OnExit ? "OnExit" : c.state == eCollisionState::OnStay ? "OnStay" : "OnCollision") << std::endl;

				switch (c.state)
				{
				case eCollisionState::OnEnter:
				{

					if (myTriggerEnterObserver)
					{
						auto* obj = myGameObject->GetScene()->FindGameObject(data.uuid);
						if (obj)
							myTriggerEnterObserver(obj);
					}
				}
				break;
				case eCollisionState::OnExit:
				{
					if (myTriggerExitObserver)
					{
						auto* obj = myGameObject->GetScene()->FindGameObject(data.uuid);
						if (obj)
							myTriggerExitObserver(obj);
					}
				}
				break;
				case eCollisionState::OnStay:
				{
					if (myTriggerStayObserver)
					{
						auto* obj = myGameObject->GetScene()->FindGameObject(data.uuid);
						if (obj)
							myTriggerStayObserver(obj);
					}
				}
				break;
				case eCollisionState::OnCollision:
				{
					if (myCollisionObserver)
					{
						auto* obj = myGameObject->GetScene()->FindGameObject(data.uuid);
						if (obj)
							myCollisionObserver(obj);
					}
				}
				break;
				}
			}
			//std::cout << "-----------" << std::endl;
		}
	}
}

void Engine::RigidBodyComponent::DebugDraw()
{
	if (!myRigidBody)
		return;
	if (!GetEngine().GetDebugDrawerSettings().IsDebugFlagActive(DebugDrawFlags::Physics))
	{
		return;
	}

	auto color = myType == eRigidBodyType::DEFAULT ? P_RIGID_COLOR :
		(myType == eRigidBodyType::KINEMATIC ? P_KINEMATIC_COLOR : P_TRIGGER_COLOR);
	for (auto& s : myShapes)
	{
		PhysicsDebugDrawer::DebugDrawShape(s, myGameObject->GetTransform().GetPosition(), myGameObject->GetTransform().GetRotation(), color);
	}

	/*
	physx::PxTransform pxTarget = myRigidBody->getGlobalPose();

	physx::PxShape** shapes = DBG_NEW physx::PxShape * [1];
	int size = myRigidBody->getShapes(shapes, 1, 0);
	if (size > 0)
	{
		CU::Vector4f color = CU::Vector4f(.4f, .4f, .7f, 1);

		if (IsCollisionListener())
		{
			auto& collisions = myGameObject->GetScene()->GetCollisions(myUUID);

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
		}
	}*/
}