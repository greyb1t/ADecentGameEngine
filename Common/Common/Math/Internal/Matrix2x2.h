#pragma once
#include <complex>
#include <iostream>

namespace Common
{
	template<class T>
	class Matrix2x2
	{
	public:
		Matrix2x2<T>();
		Matrix2x2<T>(
			const T m11, const T m12,
			const T m21, const T m22
			);
		Matrix2x2<T>(const Matrix2x2<T>& aMatrix);

		T& operator()(const int aRow, const int aColumn);
		const T& operator()(const int aRow, const int aColumn) const;

		T Minor(const int aX, const int aY);
		T Cofactor(const int aX, const int aY);
		T Determinant();
		Matrix2x2<T> Cofactors();
		Matrix2x2<T> Adjoint();
		Matrix2x2<T> Inverse();

		static Matrix2x2<T> Transpose(const Matrix2x2<T>& aMatrixToTranspose);

	private:
		T myData[2][2];
	};

	using Matrix2x2f = Matrix2x2<float>;
}