#pragma once

#include <assert.h>

namespace Common
{
	const float PI = 3.14159265359f;
	const float HALF_PI = PI * .5f;

	const float RadToDeg = 180.f / PI;
	const float DegToRad = PI / 180.f;

	template <typename T>
	T Max(const T& aA, const T& aB);

	template <typename T>
	T Min(const T& aA, const T& aB);

	template <typename T>
	T Min(const T& aA, const T& aB, const T& aC);

	template <typename T>
	T Abs(const T& aValue);

	template <typename T>
	T Clamp(const T& aMin, const T& aMax, const T& aValue);

	template <typename T>
	T Lerp(const T& aA, const T& aB, const float aT);

	template <typename T>
	T InverseLerp(const T aA, const T aB, const T aValue);

	template<typename T>
	T Remap(const T& aOrigin, const T& aOrigonTo, const T& aTargetFrom, const T& aTargetTo, const T& aValue);

	template <typename T>
	void Swap(T& aA, T& aB);

	template<typename T>
	T Max(const T& aA, const T& aB)
	{
		return (aA > aB) ? aA : aB;
	}

	template<typename T>
	T Min(const T& aA, const T& aB)
	{
		return (aA < aB) ? aA : aB;
	}

	template<typename T>
	T Min(const T& aA, const T& aB, const T& aC)
	{
		return Min(Min(aA, aB), aC);
	}

	template<typename T>
	T Abs(const T& aValue)
	{
		if (aValue < 0)
		{
			return -aValue;
		}
		else
		{
			return aValue;
		}
	}

	template<typename T>
	T Clamp(const T& aMin, const T& aMax, const T& aValue)
	{
		assert(aMax >= aMin && "Max has to be bigger than min");

		return Min(Max(aMin, aValue), aMax);
	}

	template<typename T>
	T Lerp(const T& aA, const T& aB, const float aT)
	{
		const T value = aA * (1.f - aT) + aB * aT;
		return value;
	}

	template<typename T>
	T InverseLerp(const T aA, const T aB, const T aValue)
	{
		if (aA != aB)
		{
			return Clamp(T(0), T(1), (aValue - aA) / (aB - aA));
		}
		else
		{
			return T(0);
		}

		//return (aValue - aA) / (aB - aA);
	}

	template<typename T>
	T Remap(const T& aOrigFrom, const T& aOrigTo, const T& aTargetFrom, const T& aTargetTo, const T& aValue)
	{
		float rel = static_cast<float>(InverseLerp(aOrigFrom, aOrigTo, aValue));
		return Lerp(aTargetFrom, aTargetTo, rel);
	}

	template<typename T>
	void Swap(T& aA, T& aB)
	{
		const T temp = aA;
		aA = aB;
		aB = temp;
	}

	template <typename T>
	bool AreEqual(const T aValue1, const T aValue2, const float aEpsilon = 0.0001f)
	{
		return abs(aValue1 - aValue2) < aEpsilon;
	}
}

namespace CU = Common;