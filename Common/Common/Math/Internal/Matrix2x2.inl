namespace Common
{
	template <class T>
	Matrix2x2<T>::Matrix2x2() :
		myData {
			{ 1, 0 },
			{ 0, 1 },
	}
	{
	}

	template <class T>
	Matrix2x2<T>::Matrix2x2(
		const T m11, const T m12,
		const T m21, const T m22
	) : myData {
		{ m11, m12 },
		{ m21, m22 }
	}
	{
	}

	template <class T>
	Matrix2x2<T>::Matrix2x2(const Matrix2x2<T>& aMatrix) :
		myData {
			{ aMatrix(1, 1), aMatrix(1, 2) },
			{ aMatrix(2, 1), aMatrix(2, 2) },
	}
	{
	};

	template <class T>
	T& Matrix2x2<T>::operator()(const int aRow, const int aColumn)
	{
		return myData[aRow - 1][aColumn - 1];
	}

	template <class T>
	const T& Matrix2x2<T>::operator()(const int aRow, const int aColumn) const
	{
		return myData[aRow - 1][aColumn - 1];
	}

	template <class T>
	T Matrix2x2<T>::Minor(const int aX, const int aY)
	{
		return (*this)((aY == 1) ? 2 : 1, (aX == 1) ? 2 : 1);
	}

	template <class T>
	T Matrix2x2<T>::Cofactor(const int aX, const int aY)
	{
		T result = Minor(aX, aY);

		if ((((aX - 1) ^ (aY - 1)) & 1) > 0)
		{
			result = -result;
		}

		return result;
	}

	template <class T>
	T Matrix2x2<T>::Determinant()
	{
		T result = 0;

		for (unsigned int i = 1; i <= 2; i++)
		{
			result += (*this)(1, i) * Cofactor(i, 1);
		}

		return result;
	}

	template <class T>
	Matrix2x2<T> Matrix2x2<T>::Cofactors()
	{
		Matrix2x2<T> result;

		for (unsigned int y = 1; y <= 2; y++)
		{
			for (unsigned int x = 1; x <= 2; x++)
			{
				result(x, y) = Cofactor(x, y);
			}
		}

		return result;
	}

	template <class T>
	Matrix2x2<T> Matrix2x2<T>::Adjoint()
	{
		return Matrix2x2<T>::Transpose(Cofactors());
	}

	template <class T>
	Matrix2x2<T> Matrix2x2<T>::Inverse()
	{
		return Adjoint() * (1 / Determinant());
	}

	template <class T>
	Matrix2x2<T> operator+(const Matrix2x2<T>& aMatrix0, const Matrix2x2<T>& aMatrix1)
	{
		Matrix2x2<T> result(aMatrix0);
		result += aMatrix1;
		return result;
	}

	template <class T>
	Matrix2x2<T> operator-(const Matrix2x2<T>& aMatrix0, const Matrix2x2<T>& aMatrix1)
	{
		Matrix2x2<T> result(aMatrix0);
		result -= aMatrix1;
		return result;
	}

	template <class T>
	Matrix2x2<T> operator*(const Matrix2x2<T>& aMatrix0, const Matrix2x2<T>& aMatrix1)
	{
		Matrix2x2<T> result(aMatrix0);
		result *= aMatrix1;
		return result;
	}

	template <class T>
	void operator+=(Matrix2x2<T>& aMatrix0, const Matrix2x2<T>& aMatrix1)
	{
		for (int i = 1; i <= 2; i++)
		{
			for (int j = 1; j <= 2; j++)
			{
				aMatrix0(i, j) += aMatrix1(i, j);
			}
		}
	}

	template <class T>
	void operator-=(Matrix2x2<T>& aMatrix0, const Matrix2x2<T>& aMatrix1)
	{
		for (int i = 1; i <= 2; i++)
		{
			for (int j = 1; j <= 2; j++)
			{
				aMatrix0(i, j) -= aMatrix1(i, j);
			}
		}
	}

	template <class T>
	void operator*=(Matrix2x2<T>& aMatrix0, const Matrix2x2<T>& aMatrix1)
	{
		const Matrix2x2<T> matrix0Original = aMatrix0;
		for (int i = 1; i <= 2; i++)
		{
			for (int j = 1; j <= 2; j++)
			{
				T sum = 0;
				for (int k = 1; k <= 2; k++)
				{
					sum += matrix0Original(k, j) * aMatrix1(i, k);
				}
				aMatrix0(i, j) = sum;
			}
		}
	}
}