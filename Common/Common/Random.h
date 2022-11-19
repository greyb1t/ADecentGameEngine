#pragma once

#include "Math/Math.h"

namespace Random
{
	float RandomFloat(const float aMin, const float aMax);
	int RandomInt(const int aMin, const int aMax);
	unsigned RandomUInt(unsigned aMin, unsigned aMax);
	size_t RandomUInt(const size_t aMin, const size_t aMax);

	// Returns a normalized value inside a unit sphere (just like in Unity)
	CU::Vector3f InsideUnitSphere();
}