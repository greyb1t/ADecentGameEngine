namespace Common
{
	template <class T>
	Vector4<T> operator+(const Vector4<T>& aVector0, const Vector4<T>& aVector1)
	{
		return Vector4<T>(
			aVector0.x + aVector1.x,
			aVector0.y + aVector1.y,
			aVector0.z + aVector1.z,
			aVector0.w + aVector1.w);
	}

	template <class T>
	Vector4<T> operator-(const Vector4<T>& aVector0, const Vector4<T>& aVector1)
	{
		return Vector4<T>(
			aVector0.x - aVector1.x,
			aVector0.y - aVector1.y,
			aVector0.z - aVector1.z,
			aVector0.w - aVector1.w);
	}

	template <class T>
	Vector4<T> operator*(const Vector4<T>& aVector, const T& aScalar)
	{
		return Vector4<T>(
			aVector.x * aScalar,
			aVector.y * aScalar,
			aVector.z * aScalar,
			aVector.w * aScalar);
	}

	template <class T>
	Vector4<T> operator*(const T& aScalar, const Vector4<T>& aVector)
	{
		return Vector4<T>(
			aScalar * aVector.x,
			aScalar * aVector.y,
			aScalar * aVector.z,
			aScalar * aVector.w);
	}

	template <class T>
	Vector4<T> operator/(const Vector4<T>& aVector, const T& aScalar)
	{
		return Vector4<T>(
			aVector.x / aScalar,
			aVector.y / aScalar,
			aVector.z / aScalar,
			aVector.w / aScalar);
	}

	template <class T>
	void operator+=(Vector4<T>& aVector0, const Vector4<T>& aVector1)
	{
		aVector0.x += aVector1.x;
		aVector0.y += aVector1.y;
		aVector0.z += aVector1.z;
		aVector0.w += aVector1.w;
	}

	template <class T>
	void operator-=(Vector4<T>& aVector0, const Vector4<T>& aVector1)
	{
		aVector0.x -= aVector1.x;
		aVector0.y -= aVector1.y;
		aVector0.z -= aVector1.z;
		aVector0.w -= aVector1.w;
	}

	template <class T>
	void operator*=(Vector4<T>& aVector, const T& aScalar)
	{
		aVector.x *= aScalar;
		aVector.y *= aScalar;
		aVector.z *= aScalar;
		aVector.w *= aScalar;
	}

	template <class T>
	void operator/=(Vector4<T>& aVector, const T& aScalar)
	{
		aVector.x /= aScalar;
		aVector.y /= aScalar;
		aVector.z /= aScalar;
		aVector.w /= aScalar;
	}

	template <typename T>
	bool operator==(const Vector4<T>& aLeft, const Vector4<T>& aRight)
	{
		return aLeft.x == aRight.x &&
			aLeft.y == aRight.y &&
			aLeft.z == aRight.z &&
			aLeft.w == aRight.w;
	}

	template <typename T>
	bool operator!=(const Vector4<T>& aLeft, const Vector4<T>& aRight)
	{
		return !(aLeft == aRight);
	}

	template<class T>
	inline Vector4<T>::Vector4(const T& aX, const T& aY, const T& aZ, const T& aW)
		: x(aX), y(aY), z(aZ), w(aW)
	{
	}

	template<class T>
	inline Vector4<T>::Vector4(const Vector3<T>& aVector, const float aW)
		: x(aVector.x),
		y(aVector.y),
		z(aVector.z),
		w(aW)
	{
	}

	template<class T>
	inline T Vector4<T>::LengthSqr() const
	{
		return x * x + y * y + z * z + w * w;
	}

	template<class T>
	inline T Vector4<T>::Length() const
	{
		return std::sqrt(LengthSqr());
	}

	template<class T>
	inline Vector4<T> Vector4<T>::GetNormalized() const
	{
		Vector4<T> vec = *this;

		vec.Normalize();

		return vec;
	}

	template<class T>
	inline void Vector4<T>::Normalize()
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

		if (w != 0)
		{
			w *= length;
		}
	}

	template<class T>
	inline T Vector4<T>::Dot(const Vector4<T>& aVector) const
	{
		return x * aVector.x + y * aVector.y + z * aVector.z + w * aVector.w;
	}

	template <typename T>
	template <typename T1>
	inline Vector4<T1> Vector4<T>::CastTo() const
	{
		return Vector4<T1>(static_cast<T1>(x), static_cast<T1>(y), static_cast<T1>(z), static_cast<T1>(w));
	}

	template <typename T>
	Vector3<T> Vector4<T>::ToVec3() const
	{
		return Vector3<T>(x, y, z);
	}
}