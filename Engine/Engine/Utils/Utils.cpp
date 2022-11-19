#include "pch.h"
#include "Utils.h"

void Utils::ComputeSnap(float* value, float snap)
{
	// Thank you imguizmo for this snap code

	const float snapTension = 0.5f;

	if (snap <= FLT_EPSILON)
	{
		return;
	}

	float modulo = fmodf(*value, snap);
	float moduloRatio = fabsf(modulo) / snap;
	if (moduloRatio < snapTension)
	{
		*value -= modulo;
	}
	else if (moduloRatio > (1.f - snapTension))
	{
		*value = *value - modulo + snap * ((*value < 0.f) ? -1.f : 1.f);
	}
}
