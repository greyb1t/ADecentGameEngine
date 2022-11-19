#include "pch.h"
#include "kDTreeObject.h"

kDTreeObject::kDTreeObject(const Vec3f& aPosition, const float aHalfSize)
	: myPosition(aPosition),
	myHalfSize(aHalfSize)
{
}

const Vec3f& kDTreeObject::GetPosition() const
{
	return myPosition;
}

void kDTreeObject::SetPosition(const Vec3f& aPosition)
{
	myPosition = aPosition;
}

float kDTreeObject::GetHalfSize() const
{
	return myHalfSize;
}

void kDTreeObject::Render()
{
}

void kDTreeObject::SetLitUp()
{
}

void kDTreeObject::ResetLittingUpState()
{
}
