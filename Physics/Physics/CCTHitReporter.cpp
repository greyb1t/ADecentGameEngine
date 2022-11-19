#include "pch.h"
#include "CCTHitReporter.h"

void CCTHitReporter::onShapeHit(const physx::PxControllerShapeHit& hit)
{
	physx::PxRigidDynamic* actor = hit.shape->getActor()->is<physx::PxRigidDynamic>();
	if (actor)
	{
		// CHECK THIS BEFORE ADDING FORCE SINCE IT CAN BE KINEMATIC
		//static int i = 0;
		//i++;
		//PUUID uuid = *static_cast<int*>(actor->userData);
		//std::cout << "Hit " << uuid << "   index: " << i << std::endl;
		auto flags = actor->getRigidBodyFlags();
		if (flags & physx::PxRigidBodyFlag::eKINEMATIC)
			return;

		// We only allow horizontal pushes. Vertical pushes when we stand on dynamic objects creates
		// useless stress on the solver. It would be possible to enable/disable vertical pushes on
		// particular objects, if the gameplay requires it.
		auto dir = hit.dir;
		dir.y = 0.f;
		if (dir.y == 0.0f)
		{
			physx::PxReal coeff = actor->getMass() * hit.length;
			physx::PxRigidBodyExt::addForceAtLocalPos(*actor, dir * coeff, physx::PxVec3(0, 0, 0), physx::PxForceMode::eIMPULSE);
		}
	}
}

physx::PxControllerBehaviorFlags CCTHitReporter::getBehaviorFlags(const physx::PxShape&, const physx::PxActor&)
{
	return physx::PxControllerBehaviorFlags(physx::PxControllerBehaviorFlag::eCCT_SLIDE);
}

physx::PxControllerBehaviorFlags CCTHitReporter::getBehaviorFlags(const physx::PxController&)
{
	return physx::PxControllerBehaviorFlags(physx::PxControllerBehaviorFlag::eCCT_SLIDE);
}

physx::PxControllerBehaviorFlags CCTHitReporter::getBehaviorFlags(const physx::PxObstacle&)
{
	return physx::PxControllerBehaviorFlags(physx::PxControllerBehaviorFlag::eCCT_SLIDE);
}
