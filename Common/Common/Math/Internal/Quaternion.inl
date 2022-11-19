namespace Common
{
	template<typename T>
	inline QuaternionT<T>::QuaternionT() :
		myW(1), myVector({ 0, 0, 0 })
	{
	}

	template<typename T>
	inline QuaternionT<T>::QuaternionT(const QuaternionT<T>& anOther) :
		myW(anOther.myW), myVector(anOther.myVector)
	{
	}

	template<typename T>
	inline QuaternionT<T>::QuaternionT(T aW, const Vector3<T>& aVector) :
		myW(aW), myVector(aVector)
	{
	}

	template<typename T>
	inline QuaternionT<T>::QuaternionT(const Vector3<T>& aEulerAngles)
	{
		Vector3<T> euler = aEulerAngles;

		//const float pi2 = Math::PI * 2.f;

		//euler.x = fmod(euler.x, pi2);
		//if (euler.x > Math::PI)
		//	euler.x -= pi2;
		//else if (euler.x < -Math::PI)
		//	euler.x += pi2;

		//euler.y = fmod(euler.y, pi2);
		//if (euler.y > Math::PI)
		//	euler.y -= pi2;
		//else if (euler.y < -Math::PI)
		//	euler.y += pi2;

		//euler.z = fmod(euler.z, pi2);
		//if (euler.z > Math::PI)
		//	euler.z -= pi2;
		//else if (euler.z < -Math::PI)
		//	euler.z += pi2;

		T h = euler.y * static_cast<T>(0.5);
		T p = euler.x * static_cast<T>(0.5);
		T b = euler.z * static_cast<T>(0.5);

		T sinH = sin(h);
		T sinP = sin(p);
		T sinB = sin(b);

		T cosH = cos(h);
		T cosP = cos(p);
		T cosB = cos(b);

		myW = cosH * cosP * cosB + sinH * sinP * sinB;
		myVector.x = -cosH * sinP * cosB - sinH * cosP * sinB;
		myVector.y = cosH * sinP * sinB - sinH * cosP * cosB;
		myVector.z = sinH * sinP * cosB - cosH * cosP * sinB;
	}

	template<typename T>
	inline QuaternionT<T>::QuaternionT(const Matrix4x4<T>& aM) :
		myW(0), myVector({ 0, 0, 0 })
	{
		//float tr = aM(0 + 1, 0 + 1) + aM(1 + 1, 1 + 1) + aM(2 + 1, 2 + 1) + aM(3 + 1, 3 + 1);
		//if (tr >= 1.0f) {
		//	float fourD = 2.0f * sqrt(tr);
		//	myVector.x = (aM(1 + 1, 2 + 1) - aM(2 + 1, 1 + 1)) / fourD;
		//	myVector.y = (aM(2 + 1, 0 + 1) - aM(0 + 1, 2 + 1)) / fourD;
		//	myVector.z = (aM(0 + 1, 1 + 1) - aM(1 + 1, 0 + 1)) / fourD;
		//	myW = fourD / 4.0f;
		//	return;
		//}

		//int i = 0;
		//if (aM(0 + 1, 0 + 1) <= aM(1 + 1, 1 + 1)) {
		//	i = 1;
		//}
		//if (aM(2 + 1, 2 + 1) > aM(i + 1, i + 1)) {
		//	i = 2;
		//}

		//int j = (i + 1) % 3;
		//int k = (j + 1) % 3;

		//tr = aM(i + 1, i + 1) - aM(j + 1, j + 1) - aM(k + 1, k + 1) + 1.0f;
		//float fourD = 2.0f * sqrt(tr);
		//float qa[4];

		//qa[i] = fourD / 4.0f;
		//qa[j] = (aM(j + 1, i + 1) + aM(i + 1, j + 1)) / fourD;
		//qa[k] = (aM(k + 1, i + 1) + aM(i + 1, k + 1)) / fourD;
		//qa[3] = (aM(j + 1, k + 1) - aM(k + 1, j + 1)) / fourD;

		//myVector.x = qa[0];
		//myVector.y = qa[1];
		//myVector.z = qa[2];
		//myW = qa[3];

		//////float t = aM(1, 1) + aM(2, 2) + aM(3, 3);

		//////if (t > 0.0f)
		//////{
		//////	float invS = 0.5f / sqrtf(1.0f + t);

		//////	myVector.x = (aM(3, 2) - aM(2, 3)) * invS;
		//////	myVector.y = (aM(1, 3) - aM(3, 1)) * invS;
		//////	myVector.z = (aM(2, 1) - aM(1, 2)) * invS;
		//////	myW = 0.25f / invS;
		//////}
		//////else
		//////{
		//////	if (aM(1, 1) > aM(2, 2) && aM(1, 1) > aM(3, 3))
		//////	{
		//////		float invS = 0.5f / sqrtf(1.0f + aM(1, 1) - aM(2, 2) - aM(3, 3));

		//////		myVector.x = 0.25f / invS;
		//////		myVector.y = (aM(1, 2) + aM(2, 1)) * invS;
		//////		myVector.z = (aM(3, 1) + aM(1, 3)) * invS;
		//////		myW = (aM(3, 2) - aM(2, 3)) * invS;
		//////	}
		//////	else if (aM(2, 2) > aM(3, 3))
		//////	{
		//////		float invS = 0.5f / sqrtf(1.0f + aM(2, 2) - aM(1, 1) - aM(3, 3));

		//////		myVector.x = (aM(1, 2) + aM(2, 1)) * invS;
		//////		myVector.y = 0.25f / invS;
		//////		myVector.z = (aM(2, 3) + aM(3, 2)) * invS;
		//////		myW = (aM(1, 3) - aM(3, 1)) * invS;
		//////	}
		//////	else
		//////	{
		//////		float invS = 0.5f / sqrtf(1.0f + aM(3, 3) - aM(1, 1) - aM(2, 2));

		//////		myVector.x = (aM(1, 3) + aM(3, 1)) * invS;
		//////		myVector.y = (aM(2, 3) + aM(3, 2)) * invS;
		//////		myVector.z = 0.25f / invS;
		//////		myW = (aM(2, 1) - aM(1, 2)) * invS;
		//////	}
		//////}

		T wValue = aM(1, 1) + aM(2, 2) + aM(3, 3);
		T xValue = aM(1, 1) - aM(2, 2) - aM(3, 3);
		T yValue = aM(2, 2) - aM(1, 1) - aM(3, 3);
		T zValue = aM(3, 3) - aM(1, 1) - aM(2, 2);

		unsigned biggestIndex = 0;
		T biggestValue = wValue;

		if (xValue > biggestValue)
		{
			biggestValue = xValue;
			biggestIndex = 1;
		}
		if (yValue > biggestValue)
		{
			biggestValue = yValue;
			biggestIndex = 2;
		}
		if (zValue > biggestValue)
		{
			biggestValue = zValue;
			biggestIndex = 3;
		}

		biggestValue = sqrt(biggestValue + 1.0f) * 0.5f;
		T mult = 0.25f / biggestValue;

		switch (biggestIndex)
		{
		case 0:
			myW = biggestValue;
			myVector.x = (aM(2, 3) - aM(3, 2)) * mult;
			myVector.y = (aM(3, 1) - aM(1, 3)) * mult;
			myVector.z = (aM(1, 2) - aM(2, 1)) * mult;
			break;
		case 1:
			myVector.x = biggestValue;
			myW = (aM(2, 3) - aM(3, 2)) * mult;
			myVector.y = (aM(1, 2) + aM(2, 1)) * mult;
			myVector.z = (aM(3, 1) + aM(1, 3)) * mult;
			break;
		case 2:
			myVector.y = biggestValue;
			myW = (aM(3, 1) - aM(1, 3)) * mult;
			myVector.x = (aM(1, 2) + aM(2, 1)) * mult;
			myVector.z = (aM(2, 3) + aM(3, 2)) * mult;
			break;
		case 3:
			myVector.z = biggestValue;
			myW = (aM(1, 2) - aM(2, 1)) * mult;
			myVector.x = (aM(3, 1) + aM(1, 3)) * mult;
			myVector.y = (aM(2, 3) + aM(3, 2)) * mult;
			break;
		}
	}

	template<typename T>
	inline QuaternionT<T> QuaternionT<T>::AxisAngle(CU::Vector3<T>& anAxis, T anAngle)
	{
		QuaternionT quat;
		quat.InitWithAxisAndRotation(anAxis, anAngle);
		return quat;
	}

	template<typename T>
	inline void QuaternionT<T>::InitWithAxisAndRotation(const Vector3<T>& anAxis, T aRotation)
	{
		T halfRot = aRotation * 0.5;
		myW = cos(halfRot);
		myVector = anAxis * sin(halfRot);
	}

	template<typename T>
	inline void QuaternionT<T>::Normalize()
	{
		T length = GetMagnitude();
		if (length > 0.0001)
		{
			myW = myW / length;
			myVector = myVector / length;
		}
	}

	template<typename T>
	inline QuaternionT<T> QuaternionT<T>::GetNormalized() const
	{
		/*
		T l = 1.0 / sqrt(myW * myW + myX * myX + myY * myY + myZ * myZ);
		return QuaternionT<T>(w * l, x * l, y * l, z * l);
		*/
		QuaternionT<T> returnValue(*this);
		returnValue.Normalize();
		return returnValue;
	}

	template<typename T>
	inline QuaternionT<T> QuaternionT<T>::GetInverse() const
	{
		T vectorLenghtSqr = GetMagnitude();
		if (AreEqual(vectorLenghtSqr, static_cast<T>(1.0), static_cast<T>(0.0001)))
		{
			return GetConjugate();
		}
		T denominator = sqrt(myW * myW + vectorLenghtSqr);
		return QuaternionT<T>(myW / denominator, myVector / denominator);
	}

	template<typename T>
	inline T QuaternionT<T>::GetMagnitude() const
	{
		return sqrt(myW * myW + myVector.Dot(myVector));
	}

	template<typename T>
	inline QuaternionT<T> QuaternionT<T>::GetConjugate() const
	{
		return QuaternionT<T>{myW, myVector* static_cast<T>(-1)};
	}

	template<typename T>
	inline T QuaternionT<T>::Dot(QuaternionT<T> anOther) const
	{
		T dotProduct = myW * anOther.myW + myVector.Dot(anOther.myVector);
		return dotProduct;
	}

	template<typename T>
	inline Matrix4x4<T> QuaternionT<T>::ToMatrix() const
	{
		T w = myW;
		T x = myVector.x;
		T y = myVector.y;
		T z = myVector.z;

		Matrix4x4<T> output;
		output(1, 1) = 1 - 2 * y * y - 2 * z * z;
		output(1, 2) = 2 * x * y + 2 * w * z;
		output(1, 3) = 2 * x * z - 2 * w * y;

		output(2, 1) = 2 * x * y - 2 * w * z;
		output(2, 2) = 1 - 2 * x * x - 2 * z * z;
		output(2, 3) = 2 * y * z + 2 * w * x;

		output(3, 1) = 2 * x * z + 2 * w * y;
		output(3, 2) = 2 * y * z - 2 * w * x;
		output(3, 3) = 1 - 2 * x * x - 2 * y * y;

		return output;
	}

	template<typename T>
	inline Vector3<T> QuaternionT<T>::EulerAngles() const
	{
		Vector3f euler;

		T sp = -2 * (myVector.y * myVector.z + myW * myVector.x);

		if (Abs(sp) > 0.9999)
		{
			euler.x = static_cast<T>(Math::PI * 0.5f) * sp;
			euler.y = atan2(-myVector.x * myVector.z - myW * myVector.y, static_cast<T>(0.5) - myVector.y * myVector.y - myVector.z * myVector.z);
			euler.z = 0;
		}
		else
		{
			euler.x = asin(sp);
			euler.y = atan2(myVector.x * myVector.z - myW * myVector.y, static_cast<T>(0.5) - myVector.x * myVector.x - myVector.y * myVector.y);
			euler.z = atan2(myVector.x * myVector.y - myW * myVector.z, static_cast<T>(0.5) - myVector.x * myVector.x - myVector.z * myVector.z);
		}
		return euler;
	}

	template<typename T>
	inline float QuaternionT<T>::Angle(const QuaternionT<T>& aFirst, const QuaternionT<T>& aSecond)
	{
		return (T)2 * asin((aFirst * aSecond.GetInverse()).myVector.Length());
	}

	template<typename T>
	inline QuaternionT<T> QuaternionT<T>::Slerp(const QuaternionT<T>& aFirst, QuaternionT<T> aSecond, T anInterpolation)
	{
		QuaternionT<T> returnQuaternion;
		T cosOmega = aFirst.Dot(aSecond);
		if (cosOmega < 0.0)
		{
			aSecond.myW *= -1.0;
			aSecond.myVector = aSecond.myVector * -1.f;
			cosOmega = -cosOmega;
		}

		T k0, k1;
		if (cosOmega > static_cast<T>(0.9999))
		{
			k0 = 1 - anInterpolation;
			k1 = anInterpolation;
		}
		else
		{
			T sinOmega = sqrt(1 - cosOmega * cosOmega);
			T omega = atan2(sinOmega, cosOmega);
			T oneOverSinOmega = 1 / sinOmega;

			k0 = sin((1 - anInterpolation) * omega) * oneOverSinOmega;
			k1 = sin(anInterpolation * omega) * oneOverSinOmega;
		}

		returnQuaternion.myW = aFirst.myW * k0 + aSecond.myW * k1;
		returnQuaternion.myVector = aFirst.myVector * k0 + aSecond.myVector * k1;
		returnQuaternion.Normalize();

		return returnQuaternion;
	}

	template<typename T>
	inline QuaternionT<T> QuaternionT<T>::SmoothSlerp(const QuaternionT<T>& aCurrentRotation, QuaternionT<T> aTargetRotation, T& aCurrentTime, T aTargetTime, T aDeltaTime)
	{
		if (aTargetTime < 0.00001 || aCurrentTime > aTargetTime)
		{
			return aTargetRotation;
		}

		T remainingTime = aTargetTime - aCurrentTime;
		T progress = CU::Min(1.0f, aDeltaTime / remainingTime);

		aCurrentTime += aDeltaTime; 

		return Slerp(aCurrentRotation, aTargetRotation, progress);
	}

	/*template<typename T>
	inline QuaternionT<T> operator+(const QuaternionT<T>& aFirst, const QuaternionT<T>& aSecond)
	{
		return QuaternionT<T>(aFirst.myW + aSecond.myW, aFirst.myVector + aSecond.myVector);
	}*/

	template<typename T>
	inline QuaternionT<T> operator*(const QuaternionT<T>& aFirst, const QuaternionT<T>& aSecond)
	{
		return QuaternionT<T>{aFirst.myW* aSecond.myW - aFirst.myVector.Dot(aSecond.myVector), aFirst.myW* aSecond.myVector + aSecond.myW * aFirst.myVector + aFirst.myVector.Cross(aSecond.myVector)};
	}

	template<typename T>
	inline QuaternionT<T> operator/(const QuaternionT<T>& aFirst, const QuaternionT<T>& aSecond)
	{
		return aFirst * aSecond.GetInverse();
	}

	template <typename T>
	inline bool operator!=(const QuaternionT<T>& aLeft, const QuaternionT<T>& aRight)
	{
		return aLeft.myVector != aRight.myVector || aLeft.myW != aRight.myW;
	}

	template<typename T>
	inline Vector3<T> operator*(const Vector3<T>& aVector, const QuaternionT<T>& aQuaternion)
	{
		QuaternionT<T> vectorQuaternion(0, aVector);
		return (aQuaternion * vectorQuaternion * aQuaternion.GetInverse()).myVector;
	}

	template<typename T>
	inline QuaternionT<T> operator*(const QuaternionT<T>& aQuaternion, T aScalar)
	{
		return QuaternionT<T>(aQuaternion.myW * aScalar, aQuaternion.myVector * aScalar);
	}

	template<typename T>
	QuaternionT<T> operator/(const QuaternionT<T>& aQuaternion, T aScalar)
	{
		T factor = static_cast<T>(1) / aScalar;
		return aQuaternion * factor;
	}
}