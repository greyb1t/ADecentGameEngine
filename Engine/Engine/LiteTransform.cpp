#include "pch.h"
#include "LiteTransform.h"

void Engine::LiteTransform::SetRotationLocal(const Quatf& aQuaternion)
{
	myRotation = aQuaternion;
}

void Engine::LiteTransform::SetScaleLocal(const Vec3f& aScale)
{
	myScale = aScale;
}

void Engine::LiteTransform::SetScaleLocal(const float aScale)
{
	myScale = Vec3f(aScale);
}

const Vec3f& Engine::LiteTransform::GetScaleLocal() const
{
	return myScale;
}

Mat4f Engine::LiteTransform::GetMatrix() const
{
	return Mat4f::CreateScale(GetScaleLocal())
		* GetRotationLocal().ToMatrix()
		* Mat4f::CreateTranslation(GetPositionLocal());
}

const Vec3f& Engine::LiteTransform::GetPositionLocal() const
{
	return myPosition;
}

const Quatf& Engine::LiteTransform::GetRotationLocal() const
{
	return myRotation;
}

// TODO: Optimize store forward value from earlier calls.
const Vec3f Engine::LiteTransform::LocalForward() const
{
	assert(false && "okej, denna används, inte bra, denna -1 måste bort, då får koden som använder denna att ändras på");
	auto m = GetMatrix();
	return m.GetForward() * -1.f;
}

const Vec3f Engine::LiteTransform::LocalRight() const
{
	assert(false && "okej, denna används, inte bra, denna -1 måste bort, då får koden som använder denna att ändras på");
	auto m = GetMatrix();
	return m.GetRight() * -1.f;
}

const Vec3f Engine::LiteTransform::LocalUp() const
{
	auto m = GetMatrix();
	return m.GetUp();
}

void Engine::LiteTransform::SetPositionLocal(const Vec3f& aPos)
{
	myPosition = aPos;
}