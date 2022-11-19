#include "Bezier.h"

Vec3f Bezier::QuadraticBezier(const Vec3f& aPoint0, const Vec3f& aPoint1, const Vec3f& aPoint2, float aT)
{
	aT = Math::Clamp(aT, 0.f, 1.f);

	const float oneMinusT = 1.f - aT;

	return oneMinusT * oneMinusT * aPoint0 + 2.f * oneMinusT * aT * aPoint1 + aT * aT * aPoint2;
}

Vec3f Bezier::QuadraticBezierDerivative(const Vec3f& aPoint0, const Vec3f& aPoint1, const Vec3f& aPoint2, float aT)
{
	return 2.f * (1.f - aT) * (aPoint1 - aPoint0) + 2.f * aT * (aPoint2 - aPoint1);
}

Vec3f Bezier::CubicBezier(const Vec3f& aPoint0, const Vec3f& aPoint1, const Vec3f& aPoint2, const Vec3f& aPoint3, float aT)
{
	aT = Math::Clamp(aT, 0.f, 1.f);

	const float oneMinusT = 1.f - aT;

	return oneMinusT * oneMinusT * oneMinusT * aPoint0 +
		3.f * oneMinusT * oneMinusT * aT * aPoint1 +
		3.f * oneMinusT * aT * aT * aPoint2 +
		aT * aT * aT * aPoint3;
}

Vec3f Bezier::CubicBezierDerivative(const Vec3f& aPoint0, const Vec3f& aPoint1, const Vec3f& aPoint2, const Vec3f& aPoint3, float aT)
{
	aT = Math::Clamp(aT, 0.f, 1.f);

	float oneMinusT = 1.f - aT;

	return 3.f * oneMinusT * oneMinusT * (aPoint1 - aPoint0) +
		6.f * oneMinusT * aT * (aPoint2 - aPoint1) +
		3.f * aT * aT * (aPoint3 - aPoint2);
}
