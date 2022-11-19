#include "pch.h"
#include "RTransform.h"

const Vec3f& Engine::RTransform::GetPosition() const
{
	return myPosition;
}

void Engine::RTransform::SetPosition(const Vec3f& aPosition)
{																										
	myPosition = aPosition;
}

Vec3f Engine::RTransform::GetRotationEuler() const
{
	return myRotationQuat.EulerAngles();
}

Quatf Engine::RTransform::GetRotationQuat() const
{
	return myRotationQuat;
}

void Engine::RTransform::SetRotationEuler(const Vec3f& aRotation)
{
	myRotationQuat = Quatf(aRotation);
}

void Engine::RTransform::SetRotationQuaternion(const Quatf& aQuaternion)
{
	myRotationQuat = aQuaternion;
}

const Vec3f& Engine::RTransform::GetScale() const
{
	return myScale;
}

void Engine::RTransform::SetScale(const float aScale)
{
	myScale = Vec3f(aScale, aScale, aScale);
}

void Engine::RTransform::SetScale(const Vec3f& aScale)
{
	myScale = aScale;
}

Mat4f Engine::RTransform::ToMatrix() const
{
	// MUST BE IN THIS ORDER
	// S * R * T
	// https://gamedev.stackexchange.com/questions/16719/what-is-the-correct-order-to-multiply-scale-rotation-and-translation-matrices-f

	return Mat4f::CreateScale(myScale)
		* myRotationQuat.ToMatrix()
		* Mat4f::CreateTranslation(myPosition);
}

Engine::RTransform Engine::RTransform::Combine(const RTransform& aOther) const
{
	RTransform result = *this;

	result.myPosition += aOther.myPosition;

	result.myRotationQuat = myRotationQuat * aOther.myRotationQuat;

	result.myScale.x *= aOther.myScale.x;
	result.myScale.y *= aOther.myScale.y;
	result.myScale.z *= aOther.myScale.z;

	return result;
}
