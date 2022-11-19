#include "pch.h"
#include "PhysXQueryFilter.h"

PhysXQueryFilter::PhysXQueryFilter(physx::PxU32 aMask)
{
	myMask = aMask;
}

physx::PxQueryHitType::Enum PhysXQueryFilter::preFilter(
	const physx::PxFilterData& filterData,
	const physx::PxShape* shape, 
	const physx::PxRigidActor* actor, 
	physx::PxHitFlags& queryFlags)
{
	if (shape->getQueryFilterData().word0 & myMask)
	{
		return physx::PxQueryHitType::Enum::eTOUCH;
	}

	return physx::PxQueryHitType::Enum::eNONE;
}

physx::PxQueryHitType::Enum PhysXQueryFilter::postFilter(const physx::PxFilterData& filterData,
	const physx::PxQueryHit& hit)
{
	if (hit.shape->getQueryFilterData().word0 & myMask)
	{
		return physx::PxQueryHitType::Enum::eTOUCH;
	}
	return physx::PxQueryHitType::Enum::eNONE;
}

void PhysXQueryFilter::SetMask(physx::PxU32 aMask)
{
	myMask = aMask;
}

physx::PxU32 PhysXQueryFilter::GetMask() const
{
	return myMask;
}
