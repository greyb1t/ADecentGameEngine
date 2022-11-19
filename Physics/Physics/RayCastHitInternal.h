#pragma once
#include "Types.h"

namespace physx
{
	class PxRigidActor;
}

struct RayCastHitInternal
{
	physx::PxRigidActor*	Actor = nullptr;
	physx::PxShape*			Shape = nullptr;

	CU::Vector3f			Origin;
	CU::Vector3f			Position;
	CU::Vector3f			Normal;
	float					Distance = 9999999.f;
	Layer					Layer;
};

