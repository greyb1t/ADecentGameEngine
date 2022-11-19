#pragma once

#include "Plane.hpp"
#include "Ray.hpp"
#include "AABB3D.hpp"
#include "Sphere.hpp"
#include "CommonUtilities.h"

#include <limits>

namespace Common
{
	// If the ray is parallel to the plane, aOutIntersectionPoint remains unchanged. If
	// the ray is in the plane, true is returned, if not, false is returned. If the ray
	// isn't parallel to the plane, the intersection point is stored in
	// aOutIntersectionPoint and true returned.
	template <typename T>
	bool IntersectionPlaneRay(const Plane<T>& aPlane,
		const Ray<T>& aRay, Vector3<T>& aOutIntersectionPoint);

	// If the ray intersects the AABB, true is returned, if not, false is returned.
	// A ray in one of the AABB's sides is counted as intersecting it.
	template <typename T>
	bool IntersectionAABBRay(const AABB3D<T>& aAABB, const Ray<T>& aRay);

	// If the ray intersects the sphere, true is returned, if not, false is returned.
	// A rat intersecting the surface of the sphere is considered as intersecting it.
	template <typename T>
	bool IntersectionSphereRay(const Sphere<T>& aSphere, const Ray<T>& aRay);

	template<typename T>
	bool IntersectionPlaneRay(const Plane<T>& aPlane, const Ray<T>& aRay, Vector3<T>& aOutIntersectionPoint)
	{
		const auto& rayOrigin = aRay.GetOrigin();
		const auto& planeNormal = aPlane.GetNormal();
		const auto& rayDir = aRay.GetDirection();

		const auto dirProjectedOnPlaneNormal = rayDir.Dot(planeNormal);

		// If ray direction is parallell with plane, no intersection
		if (dirProjectedOnPlaneNormal == static_cast<T>(0))
		{
			return false;
		}

		const auto originProjectedOnPlaneNormal = rayOrigin.Dot(planeNormal);
		const auto d = aPlane.GetPoint().Dot(aPlane.GetNormal());
		const T t = (d - originProjectedOnPlaneNormal) / dirProjectedOnPlaneNormal;

		if (t < static_cast<T>(0))
		{
			return false;
		}

		aOutIntersectionPoint = rayOrigin + rayDir * t;

		return true;
	}

	template<typename T>
	bool IntersectionAABBRay(const AABB3D<T>& aAABB, const Ray<T>& aRay)
	{
		// ray formula: point = ray.origin + ray.direction * t

		// re-arrange formula: t = (point - ray.origin) / ray.direction

		// Why re-arrage the formula?
		// Because we want to find the t value for which point is on the min.x and max.x

		// NOTE: The below code is probably HORRIBLY unoptimized with several branches, yikes.

		const auto& min = aAABB.GetMin();
		const auto& max = aAABB.GetMax();

		const auto& rayOrigin = aRay.GetOrigin();
		const auto& rayDir = aRay.GetDirection();

		// Start the numbers at negative and positive infinity 
		// because it makes the tests below work lol
		// P.S I hope the numeric limits infinity is a compile time number :S
		auto tNear = -std::numeric_limits<T>::infinity();
		auto tFar = std::numeric_limits<T>::infinity();

		const auto deltaMinRayOriginX = min.x - rayOrigin.x;
		const auto deltaMaxRayOriginX = max.x - rayOrigin.x;

		// Since it counts as intersection when the ray is parallell to the AABB's axis,
		// we avoid checking that axis because it would give invalid values because 0 / 0
		if (deltaMinRayOriginX != static_cast<T>(0) &&
			deltaMaxRayOriginX != static_cast<T>(0))
		{
			const auto tNearX = deltaMinRayOriginX / rayDir.x;
			const auto tFarX = deltaMaxRayOriginX / rayDir.x;

			// Find biggest min
			tNear = Max(tNear, Min(tNearX, tFarX));

			// Find smallest max
			tFar = Min(tFar, Max(tNearX, tFarX));
		}

		const auto deltaMinRayOriginY = min.y - rayOrigin.y;
		const auto deltaMaxRayOriginY = max.y - rayOrigin.y;

		if (deltaMinRayOriginY != static_cast<T>(0) &&
			deltaMaxRayOriginY != static_cast<T>(0))
		{
			const auto tNearY = deltaMinRayOriginY / rayDir.y;
			const auto tFarY = deltaMaxRayOriginY / rayDir.y;

			tNear = Max(tNear, Min(tNearY, tFarY));
			tFar = Min(tFar, Max(tNearY, tFarY));
		}

		const auto deltaMinRayOriginZ = min.z - rayOrigin.z;
		const auto deltaMaxRayOriginZ = max.z - rayOrigin.z;

		if (deltaMinRayOriginZ != static_cast<T>(0) &&
			deltaMaxRayOriginZ != static_cast<T>(0))
		{
			const auto tNearZ = deltaMinRayOriginZ / rayDir.z;
			const auto tFarZ = deltaMaxRayOriginZ / rayDir.z;

			tNear = Max(tNear, Min(tNearZ, tFarZ));
			tFar = Min(tFar, Max(tNearZ, tFarZ));
		}

		const bool isInfrontOfRay = tFar > static_cast<T>(0);

		return isInfrontOfRay && tNear < tFar;
	}

	template<typename T>
	bool IntersectionSphereRay(const Sphere<T>& aSphere, const Ray<T>& aRay)
	{
		const auto vecToSphereCenter = aSphere.GetCenter() - aRay.GetOrigin();

		// Project onto direction
		const auto vecProjectedOnDirLength = vecToSphereCenter.Dot(aRay.GetDirection());
		const auto vecProjectedOnDirLengthSq = vecProjectedOnDirLength * vecProjectedOnDirLength;

		const auto sphereRadius = aSphere.GetRadius();

		const auto radiusSq = (sphereRadius * sphereRadius);

		const auto fSq = radiusSq - vecToSphereCenter.LengthSqr() + vecProjectedOnDirLengthSq;

		// Keep for the in case needed in future...
		// const auto t = vecProjectedOnDirLength - sqrt(fSq);

		if (fSq < static_cast<T>(0))
		{
			return false;
		}

		return true;
	}
}

namespace CU = Common;