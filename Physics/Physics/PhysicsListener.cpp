#include "pch.h"
#include "PhysicsListener.h"

void PhysicsListener::onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count)
{
}

void PhysicsListener::onWake(physx::PxActor** actors, physx::PxU32 count)
{
}

void PhysicsListener::onSleep(physx::PxActor** actors, physx::PxU32 count)
{
}

void PhysicsListener::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs,
	physx::PxU32 nbPairs)
{
	auto rb1 = pairHeader.actors[0];
	auto rb2 = pairHeader.actors[1];
	//pairHeader.flags
	unsigned int* rb1ID = static_cast<unsigned int*>(rb1->userData);
	unsigned int* rb2ID = static_cast<unsigned int*>(rb2->userData);

	CollisionData data;
	data.a.uuid = rb1ID ? *rb1ID : -1;
	data.a.rigidActor = rb1;
	data.a.layer = pairs->shapes[0]->getSimulationFilterData().word0;

	data.b.uuid = rb2ID ? *rb2ID : -1;
	data.b.rigidActor = rb2;
	data.b.layer = pairs->shapes[1]->getSimulationFilterData().word0;

	myCollisions.emplace_back(data);

	auto transform1 = rb1->getGlobalPose();
	auto transform2 = rb2->getGlobalPose();

	//std::cout << "_______COLLISION_______" << std::endl;
	//
	//std::cout << "Rigidbody 1 \n" << 
	//	"	ID: (" << (rb1ID ? std::to_string(*rb1ID) : "NULLPTR") << ")\n" <<
	//	"	Transform: (X: " << transform1.p.x << " Y:" << transform1.p.y << " Z:" << //transform1.p.z << ")" << std::endl;
	//std::cout << std::endl;
	//
	//std::cout << "Rigidbody 2 \n" <<
	//	"	ID: (" << (rb2ID ? std::to_string(*rb2ID) : "NULLPTR") << ")\n" <<
	//	"	Transform: (X: " << transform2.p.x << " Y:" << transform2.p.y << " Z:" << //transform2.p.z << ")" << std::endl;
	//std::cout << std::endl;
	//
	//
	//std::cout << std::endl;
	//std::cout << std::endl;

}

void PhysicsListener::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
{
	// Only runs on enter and exit
	auto rb1 = pairs->triggerActor;
	auto rb2 = pairs->otherActor;

	unsigned int* rb1ID = static_cast<unsigned int*>(rb1->userData);
	unsigned int* rb2ID = static_cast<unsigned int*>(rb2->userData);

	CollisionData data;
	data.type = eCollisionType::TRIGGER;
	data.a.uuid = rb1ID ? *rb1ID : -1;
	//data.a.layer = pairs->triggerShape->getSimulationFilterData().word0;
	data.a.rigidActor = rb1;

	data.b.uuid = rb2ID ? *rb2ID : -1;
	//data.b.layer = pairs->otherShape->getSimulationFilterData().word0;
	data.b.rigidActor = rb2;

	myCollisions.emplace_back(data);

	//auto transform1 = rb1->getGlobalPose();
	//auto transform2 = rb2->getGlobalPose();
}

void PhysicsListener::onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer,
	const physx::PxU32 count)
{
}

void PhysicsListener::ResetCollisions()
{
	myCollisions.clear();
}

const std::vector<CollisionData>& PhysicsListener::GetCollisions() const
{
	return myCollisions;
}
