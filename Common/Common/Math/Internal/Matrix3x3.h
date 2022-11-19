#pragma once

namespace Common
{
	template <typename T>
	class Matrix3x3
	{
	public:
		static const Matrix3x3<T> Zeroed;
		static const Matrix3x3<T> Identity;

		Matrix3x3<T>();

		Matrix3x3<T>(const T(&aValues)[3 * 3]);

		Matrix3x3<T>(const Matrix3x3<T>& aMatrix);

		// Copies the top left 3x3 part of the Matrix4x4.
		//Matrix3x3<T>(const Matrix4x4<T>& aMatrix);

		static Matrix3x3<T> CreateRotationAroundX(const T aAngleInRadians);
		static Matrix3x3<T> CreateRotationAroundY(const T aAngleInRadians);
		static Matrix3x3<T> CreateRotationAroundZ(const T aAngleInRadians);

		static Matrix3x3<T> Transpose(const Matrix3x3<T>& aMatrixToTranspose);

		Matrix3x3<T>& operator=(const Matrix3x3<T>& aRhs);

		bool operator==(const Matrix3x3<T>& aRhs) const;

		T Minor(int aX, int aY) const;
		T Cofactor(int aX, int aY) const;
		T Determinant() const;
		Matrix3x3<T> Cofactors() const;
		Matrix3x3<T> Adjoint() const;
		Matrix3x3<T> Inverse() const;

		// () operator for accessing element (row, column) for read/write or read, respectively.
		T& operator()(const int aRow, const int aColumn);
		const T& operator()(const int aRow, const int aColumn) const;

		const Vector3<T>& GetRight() const;
		const Vector3<T>& GetUp() const;
		const Vector3<T>& GetForward() const;

	private:
		union
		{
			struct
			{
				// Instead of keeping in array, i keep them in variables 
				// to avoid the confusion with 0 and 1 index
				// Also avoided the "my" prefix because it would look stoopid
				T m11, m12, m13,
					m21, m22, m23,
					m31, m32, m33;
			};

			// Used for some good memcpy for performance reasons and initializations
			T myValues[3][3];

			// Added for convenience, still unsure if should be public or private, private for now
			struct
			{
				// X axis
				Vector3<T> myX;

				// Y axis
				Vector3<T> myY;

				// Z axis
				Vector3<T> myZ;
			};
		};
	};

	template <typename T>
	const Matrix3x3<T> Matrix3x3<T>::Zeroed = Matrix3x3<T>({ 0 });

	template <typename T>
	const Matrix3x3<T> Matrix3x3<T>::Identity = Matrix3x3<T>({
		1, 0, 0,
		0, 1, 0,
		0, 0, 1
		});

	using Matrix3f = Matrix3x3<float>;
	using Matrix3d = Matrix3x3<double>;
}