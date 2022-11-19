#include "pch.h"
#include "FilterCCTMove.h"

FilterCCTMove::FilterCCTMove(physx::PxU32 aMask)
{
	myMask = aMask;
}

physx::PxQueryHitType::Enum FilterCCTMove::preFilter(const physx::PxFilterData& filterData, const physx::PxShape* shape,
	const physx::PxRigidActor* actor, physx::PxHitFlags& queryFlags)
{
	if (shape->getSimulationFilterData().word0 & myMask)
	{
		return physx::PxQueryHitType::Enum::eTOUCH;
	}

	return physx::PxQueryHitType::Enum::eNONE;
}

physx::PxQueryHitType::Enum FilterCCTMove::postFilter(const physx::PxFilterData& filterData,
	const physx::PxQueryHit& hit)
{
	if (hit.shape->getSimulationFilterData().word0 & myMask)
	{
		//myHits.emplace_back(hit.actor);
		return physx::PxQueryHitType::Enum::eTOUCH;
	}
	return physx::PxQueryHitType::Enum::eNONE;
}

void FilterCCTMove::SetMask(physx::PxU32 aMask)
{
	myMask = aMask;
}

physx::PxU32 FilterCCTMove::GetMask() const
{
	return myMask;
}


//const std::vector<physx::PxRigidActor*>& FilterCCTMove::GetHits() const
//{
	//return myHits;
//}
//
//void FilterCCTMove::ClearHits()
//{
//	for (int i = 0; i < myHits.size(); i++)
//	{
//		myHits[i] = nullptr;
//	}
//	myHits.clear();
//}
