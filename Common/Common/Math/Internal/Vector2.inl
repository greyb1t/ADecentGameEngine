namespace Common
{
	template <class T>
	Vector2<T> operator+(const Vector2<T>& aVector0, const Vector2<T>& aVector1)
	{
		return Vector2<T>(
			aVector0.x + aVector1.x,
			aVector0.y + aVector1.y);
	}

	template <class T>
	Vector2<T> operator-(const Vector2<T>& aVector0, const Vector2<T>& aVector1)
	{
		return Vector2<T>(
			aVector0.x - aVector1.x,
			aVector0.y - aVector1.y);
	}

	//Returns the vector aVector multiplied by the scalar aScalar
	template <class T> Vector2<T> operator*(const Vector2<T>& aVector0, const Vector2<T>& aVector1)
	{
		return Vector2<T>(aVector0.x * aVector1.x, aVector0.y * aVector1.y);
	}

	template <class T>
	Vector2<T> operator*(const Vector2<T>& aVector, const T& aScalar)
	{
		return Vector2<T>(
			aVector.x * aScalar,
			aVector.y * aScalar);
	}

	template <class T>
	Vector2<T> operator*(const T& aScalar, const Vector2<T>& aVector)
	{
		return Vector2<T>(
			aScalar * aVector.x,
			aScalar * aVector.y);
	}

	template <class T>
	Vector2<T> operator/(const Vector2<T>& aVector, const T& aScalar)
	{
		return Vector2<T>(
			aVector.x / aScalar,
			aVector.y / aScalar);
	}

	template<class T>
	Vector2<T> operator/(const Vector2<T>& aFirst, const Vector2<T>& aSecond)
	{
		if (aSecond.x == 0 || aSecond.y == 0)
		{
			return aFirst;
		}
		return Vector2<T>(
			aFirst.x / aSecond.x,
			aFirst.y / aSecond.y
			);
	}

	template <class T>
	void operator+=(Vector2<T>& aVector0, const Vector2<T>& aVector1)
	{
		aVector0.x += aVector1.x;
		aVector0.y += aVector1.y;
	}

	template <class T>
	void operator-=(Vector2<T>& aVector0, const Vector2<T>& aVector1)
	{
		aVector0.x -= aVector1.x;
		aVector0.y -= aVector1.y;
	}

	template <class T>
	void operator*=(Vector2<T>& aVector, const T& aScalar)
	{
		aVector.x *= aScalar;
		aVector.y *= aScalar;
	}

	template <class T>
	void operator/=(Vector2<T>& aVector, const T& aScalar)
	{
		aVector.x /= aScalar;
		aVector.y /= aScalar;
	}

	template <typename T>
	bool operator==(const Vector2<T>& aLeft, const Vector2<T>& aRight)
	{
		return aLeft.x == aRight.x &&
			aLeft.y == aRight.y;
	}

	template <typename T>
	bool operator!=(const Vector2<T>& aLeft, const Vector2<T>& aRight)
	{
		return !(aLeft == aRight);
	}

	template<class T>
	inline Vector2<T>::Vector2(const T& aX, const T& aY)
		: x(aX), y(aY)
	{
	}

	template<class T>
	inline T Vector2<T>::LengthSqr() const
	{
		return x * x + y * y;
	}

	template<class T>
	inline T Vector2<T>::Length() const
	{
		return std::sqrt(LengthSqr());
	}

	template<class T>
	inline Vector2<T> Vector2<T>::GetNormalized() const
	{
		Vector2<T> vec = *this;

		vec.Normalize();

		return vec;
	}

	template<class T>
	inline void Vector2<T>::Normalize()
	{
		T length = 1.0f / Length();

		if (x != 0)
		{
			x *= length;
		}

		if (y != 0)
		{
			y *= length;
		}
	}

	template<class T>
	inline T Vector2<T>::Dot(const Vector2<T>& aVector) const
	{
		return x * aVector.x + y * aVector.y;
	}

	template <typename T>
	template <typename T1>
	inline Vector2<T1> Vector2<T>::CastTo() const
	{
		return Vector2<T1>(static_cast<T1>(x), static_cast<T1>(y));
	}

	template <typename T>
	T Vector2<T>::Distance(const Vector2<T>& aA, const Vector2<T>& aB)
	{
		return (aA - aB).Length();
	}

	template <typename T>
	T Vector2<T>::DistanceSq(const Vector2<T>& aA, const Vector2<T>& aB)
	{
		return (aA - aB).LengthSqr();
	}
}