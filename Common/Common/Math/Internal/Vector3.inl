namespace Common
{
	template <class T>
	bool operator==(const Vector3<T>& aVector0, const Vector3<T>& aVector1)
	{
		return aVector0.x == aVector1.x && aVector0.y == aVector1.y && aVector0.z == aVector1.z;
	}

	template <class T>
	bool operator!=(const Vector3<T>& aVector0, const Vector3<T>& aVector1)
	{
		return !(aVector0.x == aVector1.x && aVector0.y == aVector1.y && aVector0.z == aVector1.z);
	}

	template <class T>
	Vector3<T> operator+(const Vector3<T>& aVector0, const Vector3<T>& aVector1)
	{
		return Vector3<T>(
			aVector0.x + aVector1.x,
			aVector0.y + aVector1.y,
			aVector0.z + aVector1.z);
	}

	template <class T>
	Vector3<T> operator-(const Vector3<T>& aVector0, const Vector3<T>& aVector1)
	{
		return Vector3<T>(
			aVector0.x - aVector1.x,
			aVector0.y - aVector1.y,
			aVector0.z - aVector1.z);
	}

	template <class T>
	Vector3<T> operator*(const Vector3<T>& aVector, const T& aScalar)
	{
		return Vector3<T>(
			aVector.x * aScalar,
			aVector.y * aScalar,
			aVector.z * aScalar);
	}

	template <class T>
	Vector3<T> operator*(const T& aScalar, const Vector3<T>& aVector)
	{
		return Vector3<T>(
			aScalar * aVector.x,
			aScalar * aVector.y,
			aScalar * aVector.z);
	}

	template <class T>
	Vector3<T> operator*(const Vector3<T>& aVector, const Vector3<T>& aVector2)
	{
		return Vector3<T>(
			aVector.x * aVector2.x,
			aVector.y * aVector2.y,
			aVector.z * aVector2.z);
	}

	template <class T>
	Vector3<T> operator/(const Vector3<T>& aVector, const T& aScalar)
	{
		return Vector3<T>(
			aVector.x / aScalar,
			aVector.y / aScalar,
			aVector.z / aScalar);
	}

	template <class T>
	Vector3<T> operator/(const T& aScalar, const Vector3<T>& aVector)
	{
		return Vector3<T>(
			aScalar / aVector.x,
			aScalar / aVector.y,
			aScalar / aVector.z);
	}

	template <class T>
	Vector3<T> operator/(const Vector3<T>& aVector, const Vector3<T>& aVector2)
	{
		return Vector3<T>(
			aVector.x / aVector2.x,
			aVector.y / aVector2.y,
			aVector.z / aVector2.z);
	}

	template <class T>
	void operator+=(Vector3<T>& aVector0, const Vector3<T>& aVector1)
	{
		aVector0.x += aVector1.x;
		aVector0.y += aVector1.y;
		aVector0.z += aVector1.z;
	}

	template <class T>
	void operator-=(Vector3<T>& aVector0, const Vector3<T>& aVector1)
	{
		aVector0.x -= aVector1.x;
		aVector0.y -= aVector1.y;
		aVector0.z -= aVector1.z;
	}

	template <class T>
	void operator*=(Vector3<T>& aVector, const T& aScalar)
	{
		aVector.x *= aScalar;
		aVector.y *= aScalar;
		aVector.z *= aScalar;
	}

	template <class T>
	void operator*=(Vector3<T>& aVector, const Vector3<T>& aVector2)
	{
		aVector.x *= aVector2.x;
		aVector.y *= aVector2.y;
		aVector.z *= aVector2.z;
	}

	template <class T>
	void operator/=(Vector3<T>& aVector, const T& aScalar)
	{
		aVector.x /= aScalar;
		aVector.y /= aScalar;
		aVector.z /= aScalar;
	}

	template <class T>
	void operator/=(Vector3<T>& aVector, Vector3<T>& aVector2)
	{
		aVector.x /= aVector2.x;
		aVector.y /= aVector2.y;
		aVector.z /= aVector2.z;
	}

	template<class T>
	inline Vector3<T>::Vector3(const T& aX, const T& aY, const T& aZ)
		: x(aX), y(aY), z(aZ)
	{
	}

	template<class T>
	inline Vector3<T>::Vector3(const T& aValue)
		: x(aValue),
		y(aValue),
		z(aValue)
	{
	}

	template<class T>
	inline T Vector3<T>::LengthSqr() const
	{
		return x * x + y * y + z * z;
	}

	template<class T>
	inline T Vector3<T>::Length() const
	{
		return std::sqrt(LengthSqr());
	}

	template<class T>
	inline Vector3<T> Vector3<T>::GetNormalized() const
	{
		Vector3<T> vec = *this;

		vec.Normalize();

		return vec;
	}

	template<class T>
	inline void Vector3<T>::Normalize()
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

		if (z != 0)
		{
			z *= length;
		}
	}

	template<class T>
	inline void Vector3<T>::Normalize(T aLength)
	{
		T length = 1.0f / aLength;
		if (x != 0)
		{
			x *= length;
		}

		if (y != 0)
		{
			y *= length;
		}

		if (z != 0)
		{
			z *= length;
		}
	}

	template<class T>
	inline T Vector3<T>::Dot(const Vector3<T>& aVector) const
	{
		return x * aVector.x + y * aVector.y + z * aVector.z;
	}

	template<class T>
	inline Vector3<T> Vector3<T>::Cross(const Vector3<T>& aVector) const
	{
		return Vector3<T>(
			y * aVector.z - z * aVector.y,
			z * aVector.x - x * aVector.z,
			x * aVector.y - y * aVector.x);
	}

	template <typename T>
	Vector3<T> Vector3<T>::Scale(const Vector3<T>& aVector) const
	{
		return Vector3<T>(
			x * aVector.x,
			y * aVector.y,
			z * aVector.z);
	}

	template <typename T>
	template <typename T1>
	inline Vector3<T1> Vector3<T>::CastTo() const
	{
		return Vector3<T1>(static_cast<T1>(x), static_cast<T1>(y), static_cast<T1>(z));
	}

	template <typename T>
	Vector4<T> Vector3<T>::ToVec4(const float aW) const
	{
		return Vector4<T>(x, y, z, aW);
	}

	template <typename T>
	T Vector3<T>::Distance(const Vector3<T>& aA, const Vector3<T>& aB)
	{
		return (aA - aB).Length();
	}

	template <typename T>
	T Vector3<T>::DistanceSq(const Vector3<T>& aA, const Vector3<T>& aB)
	{
		return (aA - aB).LengthSqr();
	}
}