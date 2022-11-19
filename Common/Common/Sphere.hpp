#pragma once

#include "Common/Math/Math.h"

namespace Common
{
	template <typename T>
	class Sphere
	{
	public:
		// The radius is zero and the position is the zero vector.
		Sphere() = default;

		Sphere(const Sphere<T>& aSphere) = default;

		Sphere(const Vector3<T>& aCenter, T aRadius);

		void InitWithCenterAndRadius(const Vector3<T>& aCenter, T aRadius);

		// Returns whether a point is inside the sphere: it is inside when the point is on the
		// sphere surface or inside of the sphere.
		bool IsInside(const Vector3<T>& aPosition) const;

		const Vector3<T>& GetCenter() const;
		float GetRadius() const;

	private:
		Vector3<T> myCenter = { };
		T myRadius = { };
	};

	template<typename T>
	inline Sphere<T>::Sphere(const Vector3<T>& aCenter, T aRadius)
		: myCenter(aCenter), myRadius(aRadius)
	{
	}

	template<typename T>
	inline void Sphere<T>::InitWithCenterAndRadius(const Vector3<T>& aCenter, T aRadius)
	{
		myCenter = aCenter;
		myRadius = aRadius;
	}

	template<typename T>
	inline bool Sphere<T>::IsInside(const Vector3<T>& aPosition) const
	{
		return (aPosition - myCenter).LengthSqr() <= (myRadius * myRadius);
	}

	template<typename T>
	inline const Vector3<T>& Sphere<T>::GetCenter() const
	{
		return myCenter;
	}

	template<typename T>
	inline float Sphere<T>::GetRadius() const
	{
		return myRadius;
	}
}

namespace CU = Common;