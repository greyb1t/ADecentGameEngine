#pragma once

#include "RayCastHitInternal.h"

class RayCastHitProcessor : public physx::PxHitCallback<physx::PxRaycastHit>
{
public:
	RayCastHitProcessor() = default;
	RayCastHitProcessor(physx::PxRaycastHit* aTouches, physx::PxU32 aMaxNbTouches);

	physx::PxAgain processTouches(const physx::PxRaycastHit* buffer, physx::PxU32 nbHits) override;
private:
	friend class PhysicsScene;
	std::vector<RayCastHitInternal> myRayCastHits;
	RayCastHitInternal myClosestHit;
};

