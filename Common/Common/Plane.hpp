#pragma once

#include "Common/Math/Math.h"
#include "Sphere.hpp"

namespace Common
{
	template <typename T>
	class Plane
	{
	public:
		Plane();

		// Constructor taking three points where the normal is (aPoint1 - aPoint0) x (aPoint2 -aPoint0).
		Plane(const Vector3<T>& aPoint0, const Vector3<T>& aPoint1, const Vector3<T>& aPoint2);

		Plane(const Vector3<T>& aPoint0, const Vector3<T>& aNormal);

		// Start the plane with three points, the same as the constructor above.
		void InitWith3Points(const Vector3<T>& aPoint0, const Vector3<T>& aPoint1, const Vector3<T>& aPoint2);

		// Start the plane with a point and a normal, the same as the constructor above.
		void InitWithPointAndNormal(const Vector3<T>& aPoint, const Vector3<T>& aNormal);

		// Returns whether a point is inside the plane: it is inside when the point 
		// is on the plane or on the side the normal is pointing away from.
		bool IsInside(const Vector3<T>& aPosition) const;

		bool IsInside(const Sphere<T>& aSphere) const;

		const Vector3<T>& GetNormal() const;
		const Vector3<T>& GetPoint() const;

	private:
		Vector3<T> myPoint;
		Vector3<T> myNormal;
	};

	template<typename T>
	Plane<T>::Plane()
		: myPoint { }, myNormal { }
	{
	}

	template<typename T>
	inline Plane<T>::Plane(const Vector3<T>& aPoint0, const Vector3<T>& aPoint1, const Vector3<T>& aPoint2)
	{
		InitWith3Points(aPoint0, aPoint1, aPoint2);
	}

	template<typename T>
	inline Plane<T>::Plane(const Vector3<T>& aPoint0, const Vector3<T>& aNormal)
		: myPoint(aPoint0), myNormal(aNormal)
	{
	}

	template<typename T>
	inline void Plane<T>::InitWith3Points(const Vector3<T>& aPoint0, const Vector3<T>& aPoint1, const Vector3<T>& aPoint2)
	{
		myPoint = aPoint0;
		myNormal = (aPoint1 - aPoint0).Cross(aPoint2 - aPoint0).GetNormalized();
	}

	template<typename T>
	inline void Plane<T>::InitWithPointAndNormal(const Vector3<T>& aPoint, const Vector3<T>& aNormal)
	{
		myPoint = aPoint;
		myNormal = aNormal;
	}

	template<typename T>
	inline bool Plane<T>::IsInside(const Vector3<T>& aPosition) const
	{
		return myNormal.Dot(aPosition - myPoint) <= static_cast<T>(0);
	}

	template<typename T>
	inline bool Plane<T>::IsInside(const Sphere<T>& aSphere) const
	{
		const auto deltaSpherePlane = aSphere.GetCenter() - myPoint;

		// Is the signed distance
		const auto distanceBetweenSpherePlane = myNormal.Dot(deltaSpherePlane);

		// If the planes normal points toward the sphere
		if (distanceBetweenSpherePlane >= -aSphere.GetRadius())
		{
			return true;
		}

		return false;
	}

	template<typename T>
	inline const Vector3<T>& Plane<T>::GetNormal() const
	{
		return myNormal;
	}

	template<typename T>
	inline const Vector3<T>& Plane<T>::GetPoint() const
	{
		return myPoint;
	}
}

namespace CU = Common;