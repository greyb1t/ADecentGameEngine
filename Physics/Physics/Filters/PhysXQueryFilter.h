#pragma once

class PhysXQueryFilter
	: public physx::PxQueryFilterCallback
{
public:
	PhysXQueryFilter(physx::PxU32 aMask);

	physx::PxQueryHitType::Enum preFilter(const physx::PxFilterData& filterData, const physx::PxShape* shape, const physx::PxRigidActor* actor, physx::PxHitFlags& queryFlags) override;

	physx::PxQueryHitType::Enum postFilter(const physx::PxFilterData& filterData, const physx::PxQueryHit& hit) override;

	void			SetMask(physx::PxU32 aMask);
	physx::PxU32	GetMask() const;
private:
	physx::PxU32 myMask = 0;
};

