#include "pch.h"
#include "VelocityHandler.h"

void VelocityHandler::Set(const Vec3f& aVelocity)
{
	myVelocity = aVelocity;
}

void VelocityHandler::SetY(const float aY)
{
	myVelocity.y = aY;
}

void VelocityHandler::Add(const Vec3f& aVelocity)
{
 	myVelocity += aVelocity;
}

Vec3f& VelocityHandler::Ref()
{
	return myVelocity;
}

const Vec3f& VelocityHandler::Get() const
{
	return myVelocity;
}
