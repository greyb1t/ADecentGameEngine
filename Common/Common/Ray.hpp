#pragma once

#include "Common/Math/Math.h"

namespace Common
{
	template <typename T>
	class Ray
	{
	public:
		Ray() = default;
		Ray(const Ray<T>& aRay) = default;

		Ray(const Vector3<T>& aOrigin, const Vector3<T>& aPoint);

		void InitWith2Points(const Vector3<T>& aOrigin, const Vector3<T>& aPoint);

		void InitWithOriginAndDirection(
			const Vector3<T>& aOrigin, const Vector3<T>& aDirection);

		const Vector3<T>& GetOrigin() const;
		const Vector3<T>& GetDirection() const;

	private:
		Vector3<T> myOrigin = { };
		Vector3<T> myDirection = { };
	};

	template<typename T>
	inline Ray<T>::Ray(const Vector3<T>& aOrigin, const Vector3<T>& aPoint)
		: myOrigin(aOrigin), myDirection((aPoint - aOrigin).GetNormalized())
	{
	}

	template<typename T>
	inline void Ray<T>::InitWith2Points(const Vector3<T>& aOrigin, const Vector3<T>& aPoint)
	{
		myOrigin = aOrigin;
		myDirection = (aPoint - aOrigin).GetNormalized();
	}

	template<typename T>
	inline void Ray<T>::InitWithOriginAndDirection(const Vector3<T>& aOrigin, const Vector3<T>& aDirection)
	{
		myOrigin = aOrigin;
		myDirection = aDirection;
	}

	template<typename T>
	inline const Vector3<T>& Ray<T>::GetOrigin() const
	{
		return myOrigin;
	}

	template<typename T>
	inline const Vector3<T>& Ray<T>::GetDirection() const
	{
		return myDirection;
	}
}

namespace CU = Common;