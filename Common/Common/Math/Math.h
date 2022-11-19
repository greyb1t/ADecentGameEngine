#pragma once

// #include "Vector.hpp"

// #include "Vector3.hpp"
// #include "Vector4.hpp"

// declarations
#include "Common/Math/Internal/Vector2.h"
#include "Common/Math/Internal/Vector3.h"
#include "Common/Math/Internal/Vector4.h"
#include "Common/Math/Internal/Matrix2x2.h"
#include "Common/Math/Internal/Matrix3x3.h"
#include "Common/Math/Internal/Matrix4x4.h"
#include "Common/Math/Internal/Quaternion.h"

namespace Math
{
	constexpr float PI = 3.14159265359f;

	constexpr float RadToDeg = 180.f / PI;
	constexpr float DegToRad = PI / 180.f;

	constexpr float Deg2Rad = PI / 180.f;
	constexpr float Rad2Deg = 180.f / PI;

	float Lerp(const float aA, const float aB, const float aT);
	float LerpClamped(const float aA, const float aB, const float aT);
	float InverseLerp(const float aA, const float aB, const float aValue);
	float InverseLerpUnclamped(const float aA, const float aB, const float aValue);

	// Loops the value
	float Repeat(const float aT, const float aLength);

	CU::Vector2f Lerp(const CU::Vector2f& aA, const CU::Vector2f& aB, const float aT);
	CU::Vector3f Lerp(const CU::Vector3f& aA, const CU::Vector3f& aB, const float aT);
	CU::Vector4f Lerp(const CU::Vector4f& aA, const CU::Vector4f& aB, const float aT);

	CU::Vector3f LerpClamped(const CU::Vector3f& aA, const CU::Vector3f& aB, const float aT);

	float LerpAngleDegrees(const float aA, const float aB, const float aT);
	float LerpAngleRadians(const float aA, const float aB, const float aT);
	float InverseLerpAngleRadians(const float aA, const float aB, const float aValue);

	// Returns a value between 0 and 1
	float PerlinNoise(const float aX, const float aY);
	float PerlinNoise(const float aX, const float aY, const float aZ);

	float Clamp(const float aValue, const float aMin, const float aMax);
	int Clamp(const int aValue, const int aMin, const int aMax);

	float Remap(float value, float sourceMin, float sourceMax, float destMin, float destMax);

	CU::Vector2f RotateByAngle(const CU::Vector2f& aVector, float aAngle);
	float DirectionDiff(const CU::Vector2f& a, const CU::Vector2f& b);
	float AngleOfVector(const CU::Vector2f& a);

	float SmoothDamp(float aCurrent, float aTarget, float& aCurrentVelocity, float aSmoothTime, const float aMaxSpeed, const float aDeltaTime);
	CU::Vector3f SmoothDamp(CU::Vector3f aCurrent, CU::Vector3f aTarget, CU::Vector3f& aCurrentVelocity, float aSmoothTime, const float aMaxSpeed, const float aDeltaTime);
}

// implementations
#include "Common/Math/Internal/Vector2.inl"
#include "Common/Math/Internal/Vector3.inl"
#include "Common/Math/Internal/Vector4.inl"
#include "Common/Math/Internal/Matrix2x2.inl"
#include "Common/Math/Internal/Matrix3x3.inl"
#include "Common/Math/Internal/Matrix4x4.inl"
#include "Common/Math/Internal/Quaternion.inl"

template <typename T>
using Vec2 = Common::Vector2<T>;
template <typename T>
using Vec3 = Common::Vector3<T>;
template <typename T>
using Vec4 = Common::Vector4<T>;
using Vec2ui = Common::Vector2ui;
using Vec2f = Common::Vector2f;
using Vec3f = Common::Vector3f;
using Vec4f = Common::Vector4f;
using Quatf = Common::Quaternion;
using Mat4f = Common::Matrix4f;
using Mat3f = Common::Matrix3f;