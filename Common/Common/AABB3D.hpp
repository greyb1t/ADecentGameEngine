#pragma once

#include "Common/Math/Math.h"

namespace Common
{
	template <typename T>
	class AABB3D
	{
	public:
		AABB3D() = default;
		AABB3D(const AABB3D<T>& aAABB3D) = default;
		AABB3D(const Vector3<T>& aMin, const Vector3<T>& aMax);

		// Init the AABB with the positions of the minimum and maximum corners, same as
		// the constructor above.
		void InitWithMinAndMax(const Vector3<T>& aMin, const Vector3f& aMax);

		// Returns whether a point is inside the AABB: it is inside when the point is on any
		// of the AABB's sides or inside of the AABB.
		bool IsInside(const Vector3<T>& aPosition) const;

		const Vector3<T>& GetMin() const;
		const Vector3<T>& GetMax() const;

	private:
		Vector3<T> myMin = { };
		Vector3<T> myMax = { };
	};

	template<typename T>
	inline AABB3D<T>::AABB3D(const Vector3<T>& aMin, const Vector3<T>& aMax)
		: myMin(aMin), myMax(aMax)
	{
	}

	template<typename T>
	inline void AABB3D<T>::InitWithMinAndMax(const Vector3<T>& aMin, const Vector3f& aMax)
	{
		myMin = aMin;
		myMax = aMax;
	}

	template<typename T>
	inline bool AABB3D<T>::IsInside(const Vector3<T>& aPosition) const
	{
		return (aPosition.x >= myMin.x && aPosition.x <= myMax.x) &&
			(aPosition.y >= myMin.y && aPosition.y <= myMax.y) &&
			(aPosition.z >= myMin.z && aPosition.z <= myMax.z);
	}

	template<typename T>
	inline const Vector3<T>& AABB3D<T>::GetMin() const
	{
		return myMin;
	}

	template<typename T>
	inline const Vector3<T>& AABB3D<T>::GetMax() const
	{
		return myMax;
	}
}

namespace CU = Common;