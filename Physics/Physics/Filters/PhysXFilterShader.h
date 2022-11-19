#pragma once

inline physx::PxFilterFlags PhysicsFilter(
	physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
	physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
	physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize)
{
	if (physx::PxFilterObjectIsTrigger(attributes0) || physx::PxFilterObjectIsTrigger(attributes1))
	{
		pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;

		if (filterData0.word0 & filterData1.word2 || filterData1.word0 & filterData0.word2)
		{
			pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;
		}

		return physx::PxFilterFlag::eDEFAULT;
	}

	if (filterData0.word0 & filterData1.word1 || filterData1.word0 & filterData0.word1)
	{
		pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;

	} else
	{
		pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;
	}


	// DO FILTERING HERE

	if (filterData0.word0 & filterData1.word2 || filterData1.word0 & filterData0.word2)
	{
		pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;
	}

	return physx::PxFilterFlag::eDEFAULT;
}
