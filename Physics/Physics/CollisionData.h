#pragma once

#include "Types.h"

namespace physx
{
	class PxRigidActor;
}

enum class eCollisionType
{
	DEFAULT,
	TRIGGER,
	CCT
};

struct ColliderData
{
	// TODO: Add more collision data here
	PUUID					uuid = -1;
	unsigned int			layer;
	physx::PxRigidActor*	rigidActor;
};

struct CollisionData
{
	eCollisionType	type = eCollisionType::DEFAULT;
	eCollisionState state = eCollisionState::OnStay;
	ColliderData	a;
	ColliderData	b;

	// This is used for physics optimizing
	PUUID			other;
};