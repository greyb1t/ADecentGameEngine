#include "pch.h"
#include "RayCastHitProcessor.h"

RayCastHitProcessor::RayCastHitProcessor(physx::PxRaycastHit* aTouches, physx::PxU32 aMaxNbTouches)
	: PxHitCallback(aTouches, aMaxNbTouches)
{
}

physx::PxAgain RayCastHitProcessor::processTouches(const physx::PxRaycastHit* buffer, physx::PxU32 nbHits)
{
	int closestIndex = 0;
	float closestDistance = FLT_MAX;

	for (int i = 0; i < nbHits; i++)
	{
		auto& data = buffer[i];
		auto layer = data.shape->getQueryFilterData().word0;

		RayCastHitInternal hit;
		hit.Actor = data.actor;
		hit.Shape = data.shape;
		hit.Position = { data.position.x, data.position.y, data.position.z };
		hit.Distance = data.distance;
		hit.Normal = { data.normal.x, data.normal.y, data.normal.z };
		hit.Layer = layer;

		if (data.distance < closestDistance)
		{
			closestDistance = data.distance;
			closestIndex = i;
		}
		myRayCastHits.emplace_back(hit);
	}

	myClosestHit = myRayCastHits[closestIndex];
	return physx::PxAgain();
}