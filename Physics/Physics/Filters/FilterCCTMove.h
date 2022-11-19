#pragma once
#include <PxRigidActor.h>
#include <vector>

class FilterCCTMove
	: public physx::PxQueryFilterCallback
{
public:
	FilterCCTMove() = default;
	FilterCCTMove(physx::PxU32 aMask);

	physx::PxQueryHitType::Enum preFilter(const physx::PxFilterData& filterData, const physx::PxShape* shape, const physx::PxRigidActor* actor, physx::PxHitFlags& queryFlags) override;

	physx::PxQueryHitType::Enum postFilter(const physx::PxFilterData& filterData, const physx::PxQueryHit& hit) override;

	void			SetMask(physx::PxU32 aMask);
	physx::PxU32	GetMask() const;

	//const std::vector<physx::PxRigidActor*>&	GetHits() const;
	//
	//void										ClearHits();
private:
	//std::vector<physx::PxRigidActor*> myHits;
	physx::PxU32 myMask = 0;
};