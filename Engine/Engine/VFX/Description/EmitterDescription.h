#pragma once
#include "Common/Enums.hpp"
#include "Engine/VFX/Emitter/EmitterGeometry.h"
#include "Engine/VFX/Time/Timeline.h"

namespace VFX
{
struct EmitterDescription
{
	EmitterGeometry geometry;
	Timeline		timeline;
	Quatf			rotation;
	Vec3f			position;
	float			duration	= 1.f;
	eSpace			space		= eSpace::LOCALSPACE;
	bool			isLooping	= false;
	bool			keepAlive	= true;
};
}