#include "pch.h"
#include "VFXParticleAttraction.h"

const Vec3f& VFXParticleAttraction::GetAttractionPoint()
{
	return myAttractionPoint;
}

void VFXParticleAttraction::SetAttractionPoint(const Vec3f& aAttractionPoint)
{
	myAttractionPoint = aAttractionPoint;
}
