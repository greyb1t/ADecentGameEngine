#pragma once

#include <assert.h>
#include <cmath>

namespace Common {
template <class T> class Vector2 {
public:
    T x = T {};
    T y = T {};

    Vector2<T>() = default;

    Vector2<T>(const T& aX, const T& aY);

    Vector2<T>(const Vector2<T>& aVector) = default;

    Vector2<T>& operator=(const Vector2<T>& aVector) = default;

    bool operator<(const Vector2<T>& anOther) const
    {
        return x < anOther.x&& y < anOther.y;
    }

    ~Vector2<T>() = default;

    T LengthSqr() const;

    T Length() const;

    Vector2<T> GetNormalized() const;

    void Normalize();

    T Dot(const Vector2<T>& aVector) const;

    template <typename T1> Vector2<T1> CastTo() const;

	static T Distance(const Vector2<T>& aA, const Vector2<T>& aB);
	static T DistanceSq(const Vector2<T>& aA, const Vector2<T>& aB);
};

using Vector2f = Vector2<float>;
using Vector2d = Vector2<double>;
using Vector2ui = Vector2<unsigned int>;
}