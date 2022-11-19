#pragma once

#include <assert.h>
#include <cmath>

namespace Common
{
	template <typename T>
	class Vector4;

	template <class T>
	class Vector3
	{
	public:
		T x = T{ };
		T y = T{ };
		T z = T{ };

		Vector3<T>() = default;

		Vector3<T>(const T& aX, const T& aY, const T& aZ);

		Vector3<T>(const T& aValue);

		Vector3<T>(const Vector3<T>& aVector) = default;

		Vector3<T>& operator=(const Vector3<T>& aVector3) = default;

		~Vector3<T>() = default;

		T LengthSqr() const;

		T Length() const;

		Vector3<T> GetNormalized() const;

		void Normalize();
		void Normalize(T aLength);

		T Dot(const Vector3<T>& aVector) const;

		Vector3<T> Cross(const Vector3<T>& aVector) const;

		// Componentwise multiply (same as multiply operator overload)
		Vector3<T> Scale(const Vector3<T>& aVector) const;

		template <typename T1>
		Vector3<T1> CastTo() const;

		Vector4<T> ToVec4(const float aW) const;

		static T Distance(const Vector3<T>& aA, const Vector3<T>& aB);
		static T DistanceSq(const Vector3<T>& aA, const Vector3<T>& aB);

		static const Vector3<T> One;
	};

	template <typename T>
	const Vector3<T> Vector3<T>::One = Vector3<T>(T(1), T(1), T(1));

	using Vector3f = Vector3<float>;
	using Vector3d = Vector3<double>;
	using Vector3i = Vector3<int>;
}