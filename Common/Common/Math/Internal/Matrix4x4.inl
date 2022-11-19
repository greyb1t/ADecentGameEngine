namespace Common
{
	template<typename T>
	inline Matrix4x4<T>::Matrix4x4()
		: m11(1), m12(0), m13(0), m14(0),
		m21(0), m22(1), m23(0), m24(0),
		m31(0), m32(0), m33(1), m34(0),
		m41(0), m42(0), m43(0), m44(1)
	{
	}

	template<typename T>
	inline Matrix4x4<T>::Matrix4x4(const T(&aValues)[4 * 4])
	{
		memcpy(myValues, aValues, sizeof(myValues));
	}

	template<typename T>
	inline Matrix4x4<T>::Matrix4x4(const Matrix4x4<T>& aMatrix)
	{
		operator=(aMatrix);
	}

	template<typename T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateRotationAroundX(const T aAngleInRadians)
	{
		Matrix4x4<T> matrix = Identity;

		const auto cosTheta = cos(aAngleInRadians);
		const auto sinTheta = sin(aAngleInRadians);

		matrix.m22 = cosTheta;
		matrix.m23 = sinTheta;

		matrix.m32 = -sinTheta;
		matrix.m33 = cosTheta;

		return matrix;
	}

	template<typename T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateRotationAroundY(const T aAngleInRadians)
	{
		Matrix4x4<T> matrix = Identity;

		const auto cosTheta = cos(aAngleInRadians);
		const auto sinTheta = sin(aAngleInRadians);

		matrix.m11 = cosTheta;
		matrix.m13 = -sinTheta;

		matrix.m31 = sinTheta;
		matrix.m33 = cosTheta;

		return matrix;
	}

	template<typename T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateRotationAroundZ(const T aAngleInRadians)
	{
		Matrix4x4<T> matrix = Identity;

		const auto cosTheta = cos(aAngleInRadians);
		const auto sinTheta = sin(aAngleInRadians);

		matrix.m11 = cosTheta;
		matrix.m12 = sinTheta;

		matrix.m21 = -sinTheta;
		matrix.m22 = cosTheta;

		return matrix;
	}

	template<typename T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateTranslation(const Vector3<T> aTranslation)
	{
		auto result = Matrix4x4<T>::Identity;
		result.myW = aTranslation;

		return result;
	}

	template <typename T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateScale(const float aScale)
	{
		return Matrix4f({
			aScale, 0.f, 0.f, 0.f,
			0.f, aScale, 0.f, 0.f,
			0.f, 0.f, aScale, 0.f,
			0.f, 0.f, 0.f, 1.f
			});
	}

	template <typename T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateScale(const Vector3f& aScale)
	{
		return Matrix4f({
			aScale.x, 0.f, 0.f, 0.f,
			0.f, aScale.y, 0.f, 0.f,
			0.f, 0.f, aScale.z, 0.f,
			0.f, 0.f, 0.f, 1.f
			});
	}


	template<typename T>
	inline Matrix4x4<T> Matrix4x4<T>::Transpose(const Matrix4x4<T>& aMatrixToTranspose)
	{
		Matrix4x4<T> matrix;

		// Copy the diagonal
		matrix.m11 = aMatrixToTranspose.m11;
		matrix.m22 = aMatrixToTranspose.m22;
		matrix.m33 = aMatrixToTranspose.m33;
		matrix.m44 = aMatrixToTranspose.m44;

		// Swap the rest
		matrix.m21 = aMatrixToTranspose.m12;
		matrix.m12 = aMatrixToTranspose.m21;

		matrix.m31 = aMatrixToTranspose.m13;
		matrix.m13 = aMatrixToTranspose.m31;

		matrix.m41 = aMatrixToTranspose.m14;
		matrix.m14 = aMatrixToTranspose.m41;

		matrix.m32 = aMatrixToTranspose.m23;
		matrix.m23 = aMatrixToTranspose.m32;

		matrix.m24 = aMatrixToTranspose.m42;
		matrix.m42 = aMatrixToTranspose.m24;

		matrix.m34 = aMatrixToTranspose.m43;
		matrix.m43 = aMatrixToTranspose.m34;

		return matrix;
	}

	template <typename T>
	Matrix4x4<T> Matrix4x4<T>::LookAt(const Vector3<T>& aStart, const Vector3<T>& aTarget, const Vector3<T>& aUp)
	{
		const Vector3<T> forward = (aTarget - aStart).GetNormalized();

		// Added GetNormalized for right and up due to cascades. Might fuck shit up later on. idk
		const Vector3<T> right = aUp.Cross(forward).GetNormalized();
		const Vector3<T> up = forward.Cross(right).GetNormalized();

		Matrix4x4<T> matrix;

		matrix(1, 1) = right.x;
		matrix(1, 2) = right.y;
		matrix(1, 3) = right.z;

		matrix(2, 1) = up.x;
		matrix(2, 2) = up.y;
		matrix(2, 3) = up.z;

		matrix(3, 1) = forward.x;
		matrix(3, 2) = forward.y;
		matrix(3, 3) = forward.z;

		matrix(4, 1) = aStart.x;
		matrix(4, 2) = aStart.y;
		matrix(4, 3) = aStart.z; 

		return matrix;
	}

	template <typename T>
	T Matrix4x4<T>::Minor(const int aX, const int aY) const
	{
		Matrix3x3<T> result;

		for (int outY = 1; outY <= 3; outY++)
		{
			int inY = outY;
			if (outY >= aY)
			{
				inY++;
			}
			for (int outX = 1; outX <= 3; outX++)
			{
				int inX = outX;
				if (outX >= aX)
				{
					inX++;
				}

				result(outY, outX) = (*this)(inY, inX);
			}
		}

		return result.Determinant();
	}

	template <typename T>
	T Matrix4x4<T>::Cofactor(const int aX, const int aY) const
	{
		T result = Minor(aX, aY);

		if ((((aX - 1) ^ (aY - 1)) & 1) > 0)
		{
			result = -result;
		}

		return result;
	}

	template <typename T>
	T Matrix4x4<T>::Determinant() const
	{
		T result = 0;

		for (int i = 1; i <= 4; i++)
		{
			result += (*this)(1, i) * Cofactor(i, 1);
		}

		return result;
	}

	template <typename T>
	Matrix4x4<T> Matrix4x4<T>::Cofactors() const
	{
		Matrix4x4<T> result;

		for (unsigned int y = 1; y <= 4; y++)
		{
			for (unsigned int x = 1; x <= 4; x++)
			{
				result(y, x) = Cofactor(x, y);
			}
		}

		return result;
	}

	template <typename T>
	Matrix4x4<T> Matrix4x4<T>::Adjoint() const
	{
		return Matrix4x4<T>::Transpose(Cofactors());
	}

	template <typename T>
	Matrix4x4<T> Matrix4x4<T>::Inverse() const
	{
		return Adjoint();// **/ (1 / Determinant());
	}

	template <typename T>
	void Matrix4x4<T>::Decompose(
		Vector3<T>& aTranslation,
		QuaternionT<T>& aRotation,
		Vector3<T>& aScale) const
	{
		auto m = *this;

		aScale.x = m.myX.Length();
		aScale.y = m.myY.Length();
		aScale.z = m.myZ.Length();

		// Must normalize otherwise when scaling, the rotation gets scewed
		m.OrthoNormalize();

		aRotation = QuaternionT<T>(m);

		aTranslation = m.GetTranslation();
	}

	template <typename T>
	void Matrix4x4<T>::Decompose(
		Vector3<T>& aTranslation,
		Vector3<T>& aRotationEuler,
		Vector3<T>& aScale) const
	{
		auto m = *this;

		aScale.x = m.myX.Length();
		aScale.y = m.myY.Length();
		aScale.z = m.myZ.Length();

		// Must normalize otherwise when scaling, the rotation gets scewed
		m.OrthoNormalize();

		aRotationEuler.x = atan2f(m.myValues[1][2], m.myValues[2][2]);

		// We need to negate it because so it gives back same result as our Quaternion
		aRotationEuler.y = -atan2f(-m.myValues[0][2],
			sqrtf(m.myValues[1][2] * m.myValues[1][2] + m.myValues[2][2] * m.myValues[2][2]));

		aRotationEuler.z = atan2f(m.myValues[0][1], m.myValues[0][0]);

		aTranslation = m.GetTranslation();
	}

	template <typename T>
	Vector3<T> Matrix4x4<T>::DecomposeScale() const
	{
		Vector3<T> scale;
		scale.x = myX.Length();
		scale.y = myY.Length();
		scale.z = myZ.Length();

		return scale;
	}

	template <typename T>
	void Matrix4x4<T>::OrthoNormalize()
	{
		myX4.Normalize();
		myY4.Normalize();
		myZ4.Normalize();
	}

	template<typename T>
	inline Matrix4x4<T> Matrix4x4<T>::GetFastInverse(const Matrix4x4<T>& aTransform)
	{
		Matrix4x4<T> result = Matrix4x4<T>::Zeroed;

		// NOTE: Cannot use Matrix3x3.hpp here because of dependency recursion
		// So an ugly solution it is :(

		// Transpose the 3x3 rotation matrix part
		// Copy the diagonal
		result.m11 = aTransform.m11;
		result.m22 = aTransform.m22;
		result.m33 = aTransform.m33;

		// Swap the rest
		result.m21 = aTransform.m12;
		result.m12 = aTransform.m21;

		result.m31 = aTransform.m13;
		result.m13 = aTransform.m31;

		result.m32 = aTransform.m23;
		result.m23 = aTransform.m32;

		// Copy the right column
		result.m14 = aTransform.m14;
		result.m24 = aTransform.m24;
		result.m34 = aTransform.m34;
		result.m44 = aTransform.m44;

		const auto translation = aTransform.GetTranslation() * static_cast<T>(-1);

		const Vector3<T> newTranslationVec;
		result.m41 = (translation.x * result.m11) + (translation.y * result.m21) + (translation.z * result.m31);
		result.m42 = (translation.x * result.m12) + (translation.y * result.m22) + (translation.z * result.m32);
		result.m43 = (translation.x * result.m13) + (translation.y * result.m23) + (translation.z * result.m33);

		return result;
	}

	template<typename T>
	inline Matrix4x4<T>& Matrix4x4<T>::operator=(const Matrix4x4<T>& aRhs)
	{
		memcpy(myValues, aRhs.myValues, sizeof(myValues));
		return *this;
	}

	template<typename T>
	bool Matrix4x4<T>::operator==(const Matrix4x4<T>& aRhs) const
	{
		// NOTE: Check with epsilon?
		return m11 == aRhs.m11 && m12 == aRhs.m12 && m13 == aRhs.m13 && m14 == aRhs.m14 &&
			m21 == aRhs.m21 && m22 == aRhs.m22 && m23 == aRhs.m23 && m24 == aRhs.m24 &&
			m31 == aRhs.m31 && m32 == aRhs.m32 && m33 == aRhs.m33 && m34 == aRhs.m34 &&
			m41 == aRhs.m41 && m42 == aRhs.m42 && m43 == aRhs.m43 && m44 == aRhs.m44;
	}

	template<typename T>
	T& Matrix4x4<T>::operator()(const int aRow, const int aColumn)
	{
		assert(aRow > 0 && aColumn > 0 && "Out of bounds");
		assert(aRow <= 4 && aColumn <= 4 && "Out of bounds");

		return myValues[aRow - 1][aColumn - 1];
	}

	template<typename T>
	const T& Matrix4x4<T>::operator()(const int aRow, const int aColumn) const
	{
		assert(aRow > 0 && aColumn > 0 && "Out of bounds");
		assert(aRow <= 4 && aColumn <= 4 && "Out of bounds");

		return myValues[aRow - 1][aColumn - 1];
	}

	template<typename T>
	inline const Vector3<T>& Matrix4x4<T>::GetRight() const
	{
		return myX;
	}

	template<typename T>
	inline const Vector3<T>& Matrix4x4<T>::GetUp() const
	{
		return myY;
	}

	template<typename T>
	inline const Vector3<T>& Matrix4x4<T>::GetForward() const
	{
		return myZ;
	}

	template<typename T>
	inline const Vector3<T>& Matrix4x4<T>::GetTranslation() const
	{
		return myW;
	}

	template <typename T>
	T* Matrix4x4<T>::GetPointer()
	{
		return reinterpret_cast<T*>(myValues);
	}

	template <typename T>
	const T* Matrix4x4<T>::GetPointer() const
	{
		return reinterpret_cast<const T*>(myValues);
	}

	template<typename T>
	inline Matrix4x4<T> operator+(Matrix4x4<T> aLhs, const Matrix4x4<T>& aRhs)
	{
		aLhs += aRhs;
		return aLhs;
	}

	template<typename T>
	inline Matrix4x4<T>& operator+=(Matrix4x4<T>& aLhs, const Matrix4x4<T>& aRhs)
	{
		aLhs.m11 += aRhs.m11;
		aLhs.m12 += aRhs.m12;
		aLhs.m13 += aRhs.m13;
		aLhs.m14 += aRhs.m14;

		aLhs.m21 += aRhs.m21;
		aLhs.m22 += aRhs.m22;
		aLhs.m23 += aRhs.m23;
		aLhs.m24 += aRhs.m24;

		aLhs.m31 += aRhs.m31;
		aLhs.m32 += aRhs.m32;
		aLhs.m33 += aRhs.m33;
		aLhs.m34 += aRhs.m34;

		aLhs.m41 += aRhs.m41;
		aLhs.m42 += aRhs.m42;
		aLhs.m43 += aRhs.m43;
		aLhs.m44 += aRhs.m44;

		return aLhs;
	}

	template<typename T>
	inline Matrix4x4<T> operator-(Matrix4x4<T> aLhs, const Matrix4x4<T>& aRhs)
	{
		aLhs -= aRhs;
		return aLhs;
	}

	template<typename T>
	inline Matrix4x4<T>& operator-=(Matrix4x4<T>& aLhs, const Matrix4x4<T>& aRhs)
	{
		aLhs.m11 -= aRhs.m11;
		aLhs.m12 -= aRhs.m12;
		aLhs.m13 -= aRhs.m13;
		aLhs.m14 -= aRhs.m14;

		aLhs.m21 -= aRhs.m21;
		aLhs.m22 -= aRhs.m22;
		aLhs.m23 -= aRhs.m23;
		aLhs.m24 -= aRhs.m24;

		aLhs.m31 -= aRhs.m31;
		aLhs.m32 -= aRhs.m32;
		aLhs.m33 -= aRhs.m33;
		aLhs.m34 -= aRhs.m34;

		aLhs.m41 -= aRhs.m41;
		aLhs.m42 -= aRhs.m42;
		aLhs.m43 -= aRhs.m43;
		aLhs.m44 -= aRhs.m44;

		return aLhs;
	}

	template <class T>
	Matrix3x3<T> operator*(const Matrix3x3<T>& aMatrix0, const Matrix3x3<T>& aMatrix1)
	{
		Matrix3x3<T> result(aMatrix0);
		result *= aMatrix1;
		return result;
	}

	template<typename T>
	inline Matrix4x4<T> operator*(Matrix4x4<T> aLhs, const Matrix4x4<T>& aRhs)
	{
		aLhs *= aRhs;
		return aLhs;
	}

	template<typename T>
	inline Matrix4x4<T>& operator*=(Matrix4x4<T>& aLhs, const Matrix4x4<T>& aRhs)
	{
		Matrix4x4<T> result = Matrix4x4<T>::Zeroed;

		result.m11 = (aLhs.m11 * aRhs.m11) + (aLhs.m12 * aRhs.m21) + (aLhs.m13 * aRhs.m31) + (aLhs.m14 * aRhs.m41);
		result.m12 = (aLhs.m11 * aRhs.m12) + (aLhs.m12 * aRhs.m22) + (aLhs.m13 * aRhs.m32) + (aLhs.m14 * aRhs.m42);
		result.m13 = (aLhs.m11 * aRhs.m13) + (aLhs.m12 * aRhs.m23) + (aLhs.m13 * aRhs.m33) + (aLhs.m14 * aRhs.m43);
		result.m14 = (aLhs.m11 * aRhs.m14) + (aLhs.m12 * aRhs.m24) + (aLhs.m13 * aRhs.m34) + (aLhs.m14 * aRhs.m44);

		result.m21 = (aLhs.m21 * aRhs.m11) + (aLhs.m22 * aRhs.m21) + (aLhs.m23 * aRhs.m31) + (aLhs.m24 * aRhs.m41);
		result.m22 = (aLhs.m21 * aRhs.m12) + (aLhs.m22 * aRhs.m22) + (aLhs.m23 * aRhs.m32) + (aLhs.m24 * aRhs.m42);
		result.m23 = (aLhs.m21 * aRhs.m13) + (aLhs.m22 * aRhs.m23) + (aLhs.m23 * aRhs.m33) + (aLhs.m24 * aRhs.m43);
		result.m24 = (aLhs.m21 * aRhs.m14) + (aLhs.m22 * aRhs.m24) + (aLhs.m23 * aRhs.m34) + (aLhs.m24 * aRhs.m44);

		result.m31 = (aLhs.m31 * aRhs.m11) + (aLhs.m32 * aRhs.m21) + (aLhs.m33 * aRhs.m31) + (aLhs.m34 * aRhs.m41);
		result.m32 = (aLhs.m31 * aRhs.m12) + (aLhs.m32 * aRhs.m22) + (aLhs.m33 * aRhs.m32) + (aLhs.m34 * aRhs.m42);
		result.m33 = (aLhs.m31 * aRhs.m13) + (aLhs.m32 * aRhs.m23) + (aLhs.m33 * aRhs.m33) + (aLhs.m34 * aRhs.m43);
		result.m34 = (aLhs.m31 * aRhs.m14) + (aLhs.m32 * aRhs.m24) + (aLhs.m33 * aRhs.m34) + (aLhs.m34 * aRhs.m44);

		result.m41 = (aLhs.m41 * aRhs.m11) + (aLhs.m42 * aRhs.m21) + (aLhs.m43 * aRhs.m31) + (aLhs.m44 * aRhs.m41);
		result.m42 = (aLhs.m41 * aRhs.m12) + (aLhs.m42 * aRhs.m22) + (aLhs.m43 * aRhs.m32) + (aLhs.m44 * aRhs.m42);
		result.m43 = (aLhs.m41 * aRhs.m13) + (aLhs.m42 * aRhs.m23) + (aLhs.m43 * aRhs.m33) + (aLhs.m44 * aRhs.m43);
		result.m44 = (aLhs.m41 * aRhs.m14) + (aLhs.m42 * aRhs.m24) + (aLhs.m43 * aRhs.m34) + (aLhs.m44 * aRhs.m44);

		aLhs = result;

		return aLhs;
	}

	template<typename T>
	inline Vector4<T> operator*(const Vector4<T>& aLhs, const Matrix4x4<T>& aRhs)
	{
		Vector4<T> result;

		result.x = (aLhs.x * aRhs.m11) + (aLhs.y * aRhs.m21) + (aLhs.z * aRhs.m31) + (aLhs.w * aRhs.m41);
		result.y = (aLhs.x * aRhs.m12) + (aLhs.y * aRhs.m22) + (aLhs.z * aRhs.m32) + (aLhs.w * aRhs.m42);
		result.z = (aLhs.x * aRhs.m13) + (aLhs.y * aRhs.m23) + (aLhs.z * aRhs.m33) + (aLhs.w * aRhs.m43);
		result.w = (aLhs.x * aRhs.m14) + (aLhs.y * aRhs.m24) + (aLhs.z * aRhs.m34) + (aLhs.w * aRhs.m44);

		return result;
	}
}