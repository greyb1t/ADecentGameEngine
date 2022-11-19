#pragma once

#include <assert.h>
#include <cmath>

#include "Vector3.h"

namespace Common
{
	template <class T>
	class Vector4
	{
	public:
		T x = T{};
		T y = T{};
		T z = T{};
		T w = T{};

		Vector4<T>() = default;

		Vector4<T>(const T& aX, const T& aY, const T& aZ, const T& aW);

		Vector4<T>(const Vector3<T>& aVector, const float aW);

		Vector4<T>(const Vector4<T>& aVector) = default;

		Vector4<T>& operator=(const Vector4<T>& aVector) = default;

		void operator*=(const Vector4<T>& aVector)
		{
			x *= aVector.x;
			y *= aVector.y;
			z *= aVector.z;
			w *= aVector.w;
		}

		void operator/=(const Vector4<T>& aVector)
		{
			x /= aVector.x;
			y /= aVector.y;
			z /= aVector.z;
			w /= aVector.w;
		}

		bool operator>(const Vector4<T>& anOther) const
		{
			float lengthSqr = LengthSqr();
			float otherLengthSqr = anOther.LengthSqr();
			return lengthSqr > otherLengthSqr;
		}

		Vector4<T> Floor() const
		{
			return Vector4<T>(floor(x), floor(y), floor(z), floor(w));
		}

		bool operator<(const Vector4<T>& aVector) const
		{
			return x < aVector.x&& y < aVector.y&& z < aVector.z&& w < aVector.w;
		}



		~Vector4<T>() = default;

		T LengthSqr() const;

		T Length() const;

		Vector4<T> GetNormalized() const;

		void Normalize();

		T Dot(const Vector4<T>& aVector) const;

		template <typename T1>
		Vector4<T1> CastTo() const;

		Vector3<T> ToVec3() const;
	};

	using Vector4f = Vector4<float>;
	using Vector4d = Vector4<double>;
}