#pragma once
#include "Types.h"

struct CCTCollisionData
{
	PUUID UUID;

};

class CCTHitReporter :
	public physx::PxUserControllerHitReport,
	public physx::PxControllerBehaviorCallback
{
public:
	// Implements PxUserControllerHitReport
	virtual void								onShapeHit(const physx::PxControllerShapeHit& hit) override;
	virtual void								onControllerHit(const physx::PxControllersHit& hit) override {}
	virtual void								onObstacleHit(const physx::PxControllerObstacleHit& hit) override {}

	// Implements PxControllerBehaviorCallback
	virtual physx::PxControllerBehaviorFlags	getBehaviorFlags(const physx::PxShape&, const physx::PxActor&) override;
	virtual physx::PxControllerBehaviorFlags	getBehaviorFlags(const physx::PxController&) override;
	virtual physx::PxControllerBehaviorFlags	getBehaviorFlags(const physx::PxObstacle&) override;
private:
	unsigned int		myFilterGroup = 0;
	unsigned int		myPhysicalFilterTargets = 0;
	unsigned int		myTriggerFilterTargets = 0;
};

