#include "pch.h"
#include "RigidBody.h"

#include "Physics.h"

#include "PhysicsScene.h"
#include "Filters/PhysicsFilter.h"


RigidBody::RigidBody(eRigidBodyType aType, const CU::Vector3f& aPosition, const CU::Quaternion& aRotation, Layer aFilterGroup, LayerMask aPhysicalFilterMask, LayerMask aTriggerFilterMask)
{
	Init(aType, aPosition, aRotation, aFilterGroup, aPhysicalFilterMask, aTriggerFilterMask);
}

// Emelia was here
RigidBody::~RigidBody()
{
	if (myScene)
	{
		if (myRigidBody)
		{
			myScene->RemoveActor(this);
			if (myRigidBody->isReleasable()) 
			{
				myRigidBody->release();
			}
		}
	}

	if (myUserPUUID)
	{
		delete myUserPUUID;
		myUserPUUID = nullptr;
	}

	myScene = nullptr;
	myRigidBody = nullptr;
}

void RigidBody::SetType(eRigidBodyType aType)
{
	if (aType == myType)
		return;

	if (aType == eRigidBodyType::TRIGGER || myType == eRigidBodyType::TRIGGER)
	{
		//auto count = myRigidBody->getNbShapes();
		//physx::PxShape** shapes = DBG_NEW physx::PxShape*[count];
		//myRigidBody->getShapes(shapes, count);
		//
		//for (int i = 0; i < count; i++)
		//{
		//	myRigidBody->detachShape(*shapes[i]);
		//}
		for (auto& shape : myShapes)
		{
			shape.SetTrigger(aType == eRigidBodyType::TRIGGER);
			//myRigidBody->attachShape(*shape.myShape);
		}

		UpdateMass();

		//delete[] shapes;
	}
	
	myType = aType;
	myRigidBody->setRigidBodyFlag(
		physx::PxRigidBodyFlag::eKINEMATIC, 
		(aType == eRigidBodyType::KINEMATIC || aType == eRigidBodyType::TRIGGER));
}


void RigidBody::Attach(Shape aShape)
{
	assert(aShape.myShape != nullptr && "Warning! Tried attaching empty shape");
	
	aShape.SetupFiltering(myFilterGroup, myPhysicalFilterTargets, myDetectionFilterTargets);
	aShape.SetTrigger(myType == eRigidBodyType::TRIGGER);

	myShapes.emplace_back(aShape);
	myRigidBody->attachShape(*aShape.myShape);

	UpdateMass();
}

void RigidBody::Detach(const Shape& aShape)
{
	assert(aShape.myShape != nullptr && "Warning! Tried to detach empty shape");
	myRigidBody->detachShape(*aShape.myShape);

	UpdateMass();
}

void RigidBody::SetPosition(const CU::Vector3f& aPosition)
{
	auto t = myRigidBody->getGlobalPose();

	t.p = physx::PxVec3(physx::PxReal(aPosition.x), physx::PxReal(aPosition.y), physx::PxReal(aPosition.z));

	myRigidBody->setGlobalPose(t);
}

void RigidBody::SetRotation(const CU::Vector3f& aRotation)
{
	auto t = myRigidBody->getGlobalPose();

	t.q = physx::PxQuat(aRotation.x, aRotation.y, aRotation.z, 1.f);

	if (myType == eRigidBodyType::KINEMATIC) {
		myRigidBody->setKinematicTarget(t);
		return;
	}

	myRigidBody->setGlobalPose(t);
}

void RigidBody::SetRotation(const CU::Quaternion& aRotation)
{
	auto t = myRigidBody->getGlobalPose();

	t.q = physx::PxQuat(aRotation.myVector.x, aRotation.myVector.y, aRotation.myVector.z, aRotation.myW);

	if (myType == eRigidBodyType::KINEMATIC) {
		myRigidBody->setKinematicTarget(t);
		return;
	}

	myRigidBody->setGlobalPose(t);
}

void RigidBody::Move(const CU::Vector3f& aMovement, const CU::Quaternion& aQuaternion)
{
	assert(false && "not implemented yet bruh why tho");
	// TODO: Fix rotation movement
	
	auto t = myRigidBody->getGlobalPose();
	t.p += physx::PxVec3(physx::PxReal(aMovement.x), physx::PxReal(aMovement.y), physx::PxReal(aMovement.z));
	if (myType == eRigidBodyType::KINEMATIC)
	{
		myRigidBody->setKinematicTarget(t);
		return;
	}
	myRigidBody->setGlobalPose(t);
}

void RigidBody::Move(const CU::Vector3f& aMovement)
{
	auto t = myRigidBody->getGlobalPose();
	t.p += physx::PxVec3(physx::PxReal(aMovement.x), physx::PxReal(aMovement.y), physx::PxReal(aMovement.z));
	if (myType == eRigidBodyType::KINEMATIC)
	{
		myRigidBody->setKinematicTarget(t);
		return;
	}
	myRigidBody->setGlobalPose(t);
}

void RigidBody::SetVelocity(const CU::Vector3f& aVelocity)
{
	assert(myType != eRigidBodyType::KINEMATIC && "You're not allowed to use setLinearVelocity on a kinematic rigidbody!");
	myRigidBody->setLinearVelocity(physx::PxVec3(aVelocity.x, aVelocity.y, aVelocity.z));
}

void RigidBody::SetAngularVelocity(const CU::Vector3f& aVelocity)
{
	myRigidBody->setAngularVelocity(physx::PxVec3(aVelocity.x, aVelocity.y, aVelocity.z));
}

void RigidBody::AddForce(const CU::Vector3f& aForce, eForceMode::Enum aMode)
{
	constexpr float forceMultiplier = 1.f;

	myRigidBody->addForce(
		{aForce.x * forceMultiplier, aForce.y * forceMultiplier, aForce.z * forceMultiplier}, 
		static_cast<physx::PxForceMode::Enum>(aMode));
}

void RigidBody::AddForceAtPosition(const CU::Vector3f& aForce, const CU::Vector3f& aPosition, eForceMode::Enum aMode)
{
	constexpr float forceMultiplier = 1.f;
	physx::PxRigidBodyExt::addForceAtPos(
		*myRigidBody,
		{aForce.x * forceMultiplier, aForce.y * forceMultiplier, aForce.z * forceMultiplier},
		{ aPosition.x, aPosition.y,	aPosition.z },
		static_cast<physx::PxForceMode::Enum>(aMode));
}

void RigidBody::AddForceAtLocalPosition(const CU::Vector3f& aForce, const CU::Vector3f& aLocalPosition, eForceMode::Enum aMode)
{
	constexpr float forceMultiplier = 1.f;
	physx::PxRigidBodyExt::addForceAtPos(
		*myRigidBody,
		{aForce.x * forceMultiplier, aForce.y * forceMultiplier, aForce.z * forceMultiplier},
		{aLocalPosition.x, aLocalPosition.y, aLocalPosition.z},
		static_cast<physx::PxForceMode::Enum>(aMode));
}

CU::Vector3f RigidBody::GetVelocity() const
{
	auto velocity = myRigidBody->getLinearVelocity();
	return {velocity.x, velocity.y, velocity.z};
}

void RigidBody::SetRotationConstraints(bool x, bool y, bool z)
{
	myRigidBody->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, x);
	myRigidBody->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, y);
	myRigidBody->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, z);
}

void RigidBody::SetMass(float aMass)
{
	myMass = aMass;
	UpdateMass();
}

void RigidBody::SetupFiltering(Layer aFilterGroup, LayerMask aPhysicalFilterMask, LayerMask aDetectionFilterMask)
{
	myFilterGroup = aFilterGroup;
	myPhysicalFilterTargets = aPhysicalFilterMask;
	myDetectionFilterTargets = aDetectionFilterMask;

	for (auto& shape : myShapes)
	{
		shape.SetupFiltering(myFilterGroup, myPhysicalFilterTargets, myDetectionFilterTargets);
	}
}

void RigidBody::SetCollisionListener(bool aValue)
{
	myCollisionListener = aValue;
	if (myScene)
		myScene->GetCollisionRegistry().RegisterUUID(myUUID, aValue);
}

bool RigidBody::IsCollisionListener() const
{
	return myCollisionListener;
}

void RigidBody::ConnectUUID(PUUID aID)
{
	myUUID = aID;
	myUserPUUID = DBG_NEW PUUID(aID);
	myRigidBody->userData = myUserPUUID;

	if (myScene)
		myScene->GetCollisionRegistry().RegisterUUID(aID);
}

PUUID RigidBody::GetUUID() const
{
	return myUUID;
}

eRigidBodyType RigidBody::GetType() const
{
	return myType;
}

physx::PxRigidDynamic* RigidBody::GetActor() const
{
	return myRigidBody;
}

void RigidBody::Init(eRigidBodyType aType, const CU::Vector3f& aPosition, const CU::Quaternion& aRotation, Layer aFilterGroup, LayerMask aPhysicalFilterMask, LayerMask aDetectionFilterMask)
{
	physx::PxTransform transform(
		physx::PxVec3(physx::PxReal(aPosition.x), physx::PxReal(aPosition.y), physx::PxReal(aPosition.z)), 
		physx::PxQuat(aRotation.myVector.x, aRotation.myVector.y, aRotation.myVector.z, aRotation.myW));
	
	myRigidBody = Physics::GetInstance().GetPhysics()->createRigidDynamic(transform);

	if (!myRigidBody)
	{
		std::cout << "Rigidbody.cpp: Failed to create rigidbody.\n";
		return;
	}

	SetType(aType);
	SetupFiltering(aFilterGroup, aPhysicalFilterMask, aDetectionFilterMask);

	bool trigger = aType == eRigidBodyType::TRIGGER;
	if (aType == eRigidBodyType::KINEMATIC || trigger)
	{
		myRigidBody->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
	}
}

void RigidBody::UpdateMass()
{
	if (myShapes.size() == 0)
		return;

	bool result = physx::PxRigidBodyExt::setMassAndUpdateInertia(*myRigidBody, myMass);
}
