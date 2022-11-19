#pragma once

#include "Math.h"

namespace Bezier
{
	Vec3f QuadraticBezier(
		const Vec3f& aPoint0,
		const Vec3f& aPoint1,
		const Vec3f& aPoint2,
		float aT);

	Vec3f QuadraticBezierDerivative(
		const Vec3f& aPoint0,
		const Vec3f& aPoint1,
		const Vec3f& aPoint2,
		float aT);

	Vec3f CubicBezier(
		const Vec3f& aPoint0,
		const Vec3f& aPoint1,
		const Vec3f& aPoint2,
		const Vec3f& aPoint3,
		float aT);

	Vec3f CubicBezierDerivative(
		const Vec3f& aPoint0,
		const Vec3f& aPoint1,
		const Vec3f& aPoint2,
		const Vec3f& aPoint3,
		float aT);
}