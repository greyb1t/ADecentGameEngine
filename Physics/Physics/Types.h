#pragma once

typedef int PUUID;

typedef unsigned int Layer;
typedef unsigned int LayerMask;

enum class eCollisionState 
{
	OnEnter,
	OnStay,
	OnExit,
	OnCollision,
};

struct eForceMode
{
	enum Enum
	{
		FORCE,				//!< parameter has unit of mass * distance/ time^2, i.e. a force
		IMPULSE,			//!< parameter has unit of mass * distance /time
		VELOCITY_CHANGE,	//!< parameter has unit of distance / time, i.e. the effect is mass independent: a velocity change.
		ACCELERATION		//!< parameter has unit of distance/ time^2, i.e. an acceleration. It gets treated just like a force except the mass is not divided out before integration.
	};
};