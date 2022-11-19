#include "Math.h"
#include "PerlinNoise.h"
#include "Common/Random.h"
#include <algorithm>

CU::Vector2f Math::Lerp(const CU::Vector2f& aA, const CU::Vector2f& aB, const float aT)
{
	return {
		Lerp(aA.x, aB.x, aT),
		Lerp(aA.y, aB.y, aT),
	};
}

CU::Vector3f Math::LerpClamped(const CU::Vector3f& aA, const CU::Vector3f& aB, const float aT)
{
	return {
		LerpClamped(aA.x, aB.x, aT),
		LerpClamped(aA.y, aB.y, aT),
		LerpClamped(aA.z, aB.z, aT),
	};
}

float Math::LerpClamped(const float aA, const float aB, const float aT)
{
	const auto value = aA + (aB - aA) * aT;
	if (aA < aB)
	{
		return Math::Clamp(value, aA, aB);
	}
	else
	{
		return Math::Clamp(value, aB, aA);
	}
}

float Math::Remap(float value, float sourceMin, float sourceMax, float destMin, float destMax)
{
	return destMin + ((value - sourceMin) / (sourceMax - sourceMin)) * (destMax - destMin);
}
float Math::LerpAngleDegrees(const float aA, const float aB, const float aT)
{
	float deltaAngle = aB - aA;

	if (deltaAngle > 180.f)
	{
		deltaAngle = deltaAngle - 360.f;
	}
	else if (deltaAngle < -180.f)
	{
		deltaAngle = deltaAngle + 360.f;
	}

	const float result = Lerp(aA, aA + deltaAngle, aT);

	if (result >= 360.f)
	{
		return result - 360.f;
	}
	else if (result < 0.f)
	{
		return result + 360.f;
	}
	else
	{
		return result;
	}
}

float Math::LerpAngleRadians(const float aA, const float aB, const float aT)
{
	float deltaAngle = aB - aA;

	if (deltaAngle > Math::PI)
	{
		deltaAngle = deltaAngle - Math::PI * 2.f;
	}
	else if (deltaAngle < -Math::PI)
	{
		deltaAngle = deltaAngle + Math::PI * 2.f;
	}

	const float result = Lerp(aA, aA + deltaAngle, aT);

	if (result >= Math::PI * 2.f)
	{
		return result - Math::PI * 2.f;
	}
	else if (result < 0.f)
	{
		return result + Math::PI * 2.f;
	}
	else
	{
		return result;
	}
}

float Math::InverseLerpAngleRadians(const float aA, const float aB, const float aValue)
{
	float deltaAngle = aB - aA;

	if (deltaAngle > Math::PI)
	{
		deltaAngle = deltaAngle - Math::PI * 2.f;
	}
	else if (deltaAngle < -Math::PI)
	{
		deltaAngle = deltaAngle + Math::PI * 2.f;
	}

	const float result = InverseLerp(aA, aA + deltaAngle, aValue);

	return result;
}

static PerlinNoise globalPerlinNoise(Random::RandomInt(1, 99999999));

float Math::PerlinNoise(const float aX, const float aY)
{
	return static_cast<float>(globalPerlinNoise.noise(aX, aY, 0.f));
}
float Math::PerlinNoise(const float aX, const float aY, const float aZ)
{
	return static_cast<float>(globalPerlinNoise.noise(aX, aY, aZ));
}

float Math::Clamp(const float aValue, const float aMin, const float aMax)
{
	return std::clamp(aValue, aMin, aMax);
}

int Math::Clamp(const int aValue, const int aMin, const int aMax)
{
	return std::clamp(aValue, aMin, aMax);
}

CU::Vector2f Math::RotateByAngle(const CU::Vector2f& aVector, float aAngle)
{
	aAngle = Deg2Rad * aAngle;
	return CU::Vector2f(cosf(aAngle) * aVector.x - sinf(aAngle) * aVector.y, cosf(aAngle) * aVector.y + sinf(aAngle) * aVector.x);
}

float Math::DirectionDiff(const CU::Vector2f& a, const CU::Vector2f& b)
{
	return Rad2Deg * (acos(a.Dot(b) / (a.Length() * b.Length())));
}

float Math::AngleOfVector(const CU::Vector2f& a)
{
	return atan2(a.y, a.x) * Rad2Deg;
}


CU::Vector3f Math::Lerp(const CU::Vector3f& aA, const CU::Vector3f& aB, const float aT)
{
	return {
		Lerp(aA.x, aB.x, aT),
		Lerp(aA.y, aB.y, aT),
		Lerp(aA.z, aB.z, aT),
	};
}

CU::Vector4f Math::Lerp(const CU::Vector4f& aA, const CU::Vector4f& aB, const float aT)
{
	return {
		Lerp(aA.x, aB.x, aT),
		Lerp(aA.y, aB.y, aT),
		Lerp(aA.z, aB.z, aT),
		Lerp(aA.w, aB.w, aT)
	};
}

float Math::Lerp(const float aA, const float aB, const float aT)
{
	const auto value = aA + (aB - aA) * aT;
	return value;
}

float Math::InverseLerp(const float aA, const float aB, const float aValue)
{
	if (aA != aB)
	{
		return Clamp((aValue - aA) / (aB - aA), 0.f, 1.f);
	}
	else
	{
		return 0.f;
	}

	//return (aValue - aA) / (aB - aA);
}

float Math::InverseLerpUnclamped(const float aA, const float aB, const float aValue)
{
	return (aValue - aA) / (aB - aA);
}

float Math::Repeat(const float aT, const float aLength)
{
	return Clamp(aT - std::floor(aT / aLength) * aLength, 0.f, aLength);
}

float Math::SmoothDamp(float aCurrent, float aTarget, float& aCurrentVelocity, float aSmoothTime, const float aMaxSpeed, const float aDeltaTime)
{
	aSmoothTime = std::max(0.0001f, aSmoothTime);
	float num = 2.f / aSmoothTime;
	float num2 = num * aDeltaTime;
	float num3 = 1.f / (1.f + num2 + 0.48f * num2 * num2 + 0.235f * num2 * num2 * num2);
	float num4 = aCurrent - aTarget;
	float num5 = aTarget;
	float num6 = aMaxSpeed * aSmoothTime;
	num4 = Math::Clamp(num4, -num6, num6);
	aTarget = aCurrent - num4;
	float num7 = (aCurrentVelocity + num * num4) * aDeltaTime;
	aCurrentVelocity = (aCurrentVelocity - num * num7) * num3;
	float num8 = aTarget + (num4 + num7) * num3;
	if (num5 - aCurrent > 0.f == num8 > num5)
	{
		num8 = num5;
		aCurrentVelocity = (num8 - num5) / aDeltaTime;
	}
	return num8;
}

CU::Vector3f Math::SmoothDamp(CU::Vector3f aCurrent, CU::Vector3f aTarget, CU::Vector3f& aCurrentVelocity, float aSmoothTime, const float aMaxSpeed, const float aDeltaTime)
{
	const float x = Math::SmoothDamp(aCurrent.x, aTarget.x, aCurrentVelocity.x, aSmoothTime, aMaxSpeed, aDeltaTime);
	const float y = Math::SmoothDamp(aCurrent.y, aTarget.y, aCurrentVelocity.y, aSmoothTime, aMaxSpeed, aDeltaTime);
	const float z = Math::SmoothDamp(aCurrent.z, aTarget.z, aCurrentVelocity.z, aSmoothTime, aMaxSpeed, aDeltaTime);
	return CU::Vector3f(x, y, z);
}
