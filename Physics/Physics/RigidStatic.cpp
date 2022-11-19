#include "pch.h"
#include "RigidStatic.h"

#include "Physics.h"
#include "PhysicsScene.h"

RigidStatic::RigidStatic(
	const CU::Vector3f&		aPosition, 
	const CU::Quaternion&	aRotation, 
	Layer					aFilterGroup, 
	LayerMask				aPhysicalFilterMask, 
	LayerMask				aDetectionMask)
{
	Init(aPosition, aRotation, aFilterGroup, aPhysicalFilterMask, aDetectionMask);
}

RigidStatic::~RigidStatic()
{
	if (myScene)
	{
		if (myRigidStatic)
		{
			myScene->RemoveActor(this);
		}
	}

	myScene = nullptr;
	myRigidStatic = nullptr;

	if (myUserPUUID)
	{
		delete myUserPUUID;
		myUserPUUID = nullptr;
	}
}

bool RigidStatic::Init(
	const CU::Vector3f& aPosition,
	const CU::Quaternion& aRotation,
	Layer aFilterGroup,
	LayerMask aPhysicalFilterMask,
	LayerMask aDetectionMask)
{
	physx::PxTransform transform(
		{aPosition.x, aPosition.y, aPosition.z},
		physx::PxQuat(aRotation.myVector.x, aRotation.myVector.y, aRotation.myVector.z, aRotation.myW));

	myRigidStatic = Physics::GetInstance().GetPhysics()->createRigidStatic(transform);

	if (!myRigidStatic)
		return false;

	SetupFiltering(aFilterGroup, aPhysicalFilterMask, aDetectionMask);
	return true;
}

void RigidStatic::Attach(Shape aShape)
{
	assert(aShape.myShape != nullptr && "Warning! Tried attaching empty shape");

	// Should add check so we can only add filters to not global shapes
	aShape.SetupFiltering(myFilterGroup, myPhysicalFilterTargets, myDetectionFilterTargets);

	myShapes.emplace_back(aShape);
	myRigidStatic->attachShape(*aShape.myShape);
}

void RigidStatic::SetPosition(const CU::Vector3f& aPosition)
{
	auto t = myRigidStatic->getGlobalPose();

	t.p = physx::PxVec3(physx::PxReal(aPosition.x), physx::PxReal(aPosition.y), physx::PxReal(aPosition.z));
	myRigidStatic->setGlobalPose(t);
}

void RigidStatic::SetTransform(const CU::Vector3f& aPosition, const CU::Quaternion& aRotation)
{
	auto t = myRigidStatic->getGlobalPose();

	t.p = physx::PxVec3(physx::PxReal(aPosition.x), physx::PxReal(aPosition.y), physx::PxReal(aPosition.z));
	auto euler = aRotation.EulerAngles();
	t.q = physx::PxQuat(euler.x, euler.y, euler.z, aRotation.myW);
	myRigidStatic->setGlobalPose(t);
}

void RigidStatic::Move(float x, float y, float z)
{
	auto t = myRigidStatic->getGlobalPose();
	t.p += physx::PxVec3(physx::PxReal(x), physx::PxReal(y), physx::PxReal(z));
	myRigidStatic->setGlobalPose(t);
}

void RigidStatic::SetTrigger(bool aValue)
{
	unsigned int shapesSize = myRigidStatic->getNbShapes();

	physx::PxShape** shapes = DBG_NEW physx::PxShape*[shapesSize];
	int size = myRigidStatic->getShapes(shapes, shapesSize);
	for (int i = 0; i < size; i++)
	{
		shapes[i]->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !aValue);
		shapes[i]->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, aValue);
	}
	delete shapes;
	shapes = nullptr;
}

bool RigidStatic::IsTrigger() const
{
	unsigned int shapesSize = myRigidStatic->getNbShapes();

	physx::PxShape** shapes = DBG_NEW physx::PxShape * [shapesSize];
	int size = myRigidStatic->getShapes(shapes, shapesSize);
	for (int i = 0; i < size; i++)
	{
		physx::PxShapeFlags flags = shapes[i]->getFlags();

		if (!(flags & physx::PxShapeFlag::eTRIGGER_SHAPE))
		{
			delete[] shapes;
			shapes = nullptr;
			return false;
		}
	}
	delete[] shapes;
	shapes = nullptr;
	return true;
}

void RigidStatic::SetupFiltering(Layer aFilterGroup, LayerMask aPhysicalFilterMask, LayerMask aDetectionLayerMask)
{
	myFilterGroup = aFilterGroup;
	myPhysicalFilterTargets = aPhysicalFilterMask;
	myDetectionFilterTargets = aDetectionLayerMask;

	for (auto& shape : myShapes)
	{
		shape.SetupFiltering(myFilterGroup, myPhysicalFilterTargets, aDetectionLayerMask);
	}
}

void RigidStatic::SetCollisionListener(bool aValue)
{
	myCollisionListener = aValue;
	if (myScene)
		myScene->GetCollisionRegistry().RegisterUUID(myUUID, aValue);
}

bool RigidStatic::IsCollisionListener() const
{
	return myCollisionListener;
}

void RigidStatic::ConnectUUID(PUUID aID)
{
	myUUID = aID;
	myUserPUUID = DBG_NEW PUUID(aID);
	myRigidStatic->userData = myUserPUUID;
}

PUUID RigidStatic::GetUUID() const
{
	return myUUID;
}

physx::PxRigidStatic* RigidStatic::GetActor() const
{
	return myRigidStatic;
}
