#pragma once

namespace Common
{
	template <typename T>
	class QuaternionT;

	template <typename T>
	class Matrix4x4
	{
	public:
		static const Matrix4x4<T> Zeroed;
		static const Matrix4x4<T> Identity;

		Matrix4x4<T>();

		Matrix4x4<T>(const T(&aValues)[4 * 4]);

		Matrix4x4<T>(const Matrix4x4<T>& aMatrix);

		static Matrix4x4<T> CreateRotationAroundX(const T aAngleInRadians);
		static Matrix4x4<T> CreateRotationAroundY(const T aAngleInRadians);
		static Matrix4x4<T> CreateRotationAroundZ(const T aAngleInRadians);

		static Matrix4x4<T> CreateTranslation(const Vector3<T> aTranslation);
		static Matrix4x4<T> CreateScale(const float aScale);
		static Matrix4x4<T> CreateScale(const Vector3f& aScale);

		static Matrix4x4<T> Transpose(const Matrix4x4<T>& aMatrixToTranspose);

		static Matrix4x4<T> LookAt(const Vector3<T>& aStart, const Vector3<T>& aTarget, const Vector3<T>& aUp = Vector3<float>(0.0f, 1.0f, 0.0f));
		
		T Minor(const int aX, const int aY) const;
		T Cofactor(const int aX, const int aY) const;
		T Determinant() const;
		Matrix4x4<T> Cofactors() const;
		Matrix4x4<T> Adjoint() const;
		Matrix4x4<T> Inverse() const;

		void Decompose(Vector3<T>& aTranslation, QuaternionT<T>& aRotation, Vector3<T>& aScale) const;
		void Decompose(Vector3<T>& aTranslation, Vector3<T>& aRotationEuler, Vector3<T>& aScale) const;
		Vector3<T> DecomposeScale() const;


		void OrthoNormalize();

		// Assumes aTransform is made up of nothing but rotations and translations.
		static Matrix4x4<T> GetFastInverse(const Matrix4x4<T>& aTransform);

		Matrix4x4<T>& operator=(const Matrix4x4<T>& aRhs);

		bool operator==(const Matrix4x4<T>& aRhs) const;

		// () operator for accessing element (row, column) for read/write or read, respectively.
		T& operator()(const int aRow, const int aColumn);
		const T& operator()(const int aRow, const int aColumn) const;

		static Matrix4x4<T> CreateOrthographic(float aLeft, float aRight, float aBot, float aTop, float aNear, float aFar)
		{
			// Taken from microsofts matrices. Not sure if theyre right for us.
			//Matrix4x4<T> mat{};
			//
			//float recipWidth = 1.f / (aRight - aLeft);
			//float recipHeight = 1.f / (aTop - aBot);
			//float range = 1.f / (aFar - aNear);
			//
			//mat(1, 1) = recipWidth + recipWidth;
			//mat(1, 2) = 0.f;
			//mat(1, 3) = 0.f;
			//mat(1, 4) = 0.f;
			//
			//mat(2, 1) = 0.f;
			//mat(2, 2) = recipHeight + recipHeight;
			//mat(2, 3) = 0.f;
			//mat(2, 4) = 0.f;
			//
			//mat(3, 1) = 0.f;
			//mat(3, 2) = 0.f;
			//mat(3, 3) = range;
			//mat(3, 4) = 0.f;
			//
			//mat(4, 1) = -(aLeft + aRight) * recipWidth;
			//mat(4, 2) = -(aTop + aBot) * recipHeight;
			//mat(4, 3) = -range * aNear;
			//mat(4, 4) = 1.f;
			//return mat;

			//Working ones I think.
			Vector2<T> res = { aRight - aLeft, aTop - aBot };

			Matrix4x4<T> mat{};
			mat(1, 1) = 2.f / res.x;
			mat(2, 2) = 2.f / res.y;
			mat(3, 3) = 1.f / (aFar - aNear);
			mat(4, 3) = aNear / (aNear - aFar);
			mat(4, 4) = 1.f;
			return mat;
		}

		void InitOrthographic(float aLeft, float aRight, float aBot, float aTop, float aNear, float aFar)
		{
			assert(false && "This doesnt work. Bad luck I guess.");
			float reciprocalWidth = 1.0f / (aRight - aLeft);
			float reciprocalHeight = 1.0f / (aTop - aBot);
			float range = 1.0f / (aFar - aNear);

			myValues[0][0] = reciprocalWidth + reciprocalWidth;
			myValues[0][1] = 0.0f;
			myValues[0][2] = 0.0f;
			myValues[0][3] = 0.0f;

			myValues[1][0] = 0.0f;
			myValues[1][1] = reciprocalHeight + reciprocalHeight;
			myValues[1][2] = 0.0f;
			myValues[1][3] = 0.0f;

			myValues[2][0] = 0.0f;
			myValues[2][1] = 0.0f;
			myValues[2][2] = range;
			myValues[2][3] = 0.0f;

			myValues[3][0] = -(aLeft + aRight) * reciprocalWidth;
			myValues[3][1] = -(aTop + aBot) * reciprocalHeight;
			myValues[3][2] = -range * aNear;
			myValues[3][3] = 1.0f;
		}

		void InitPerspective(const float aFoV, const Vector2<float>& aResolution, const float aNearPlane, const float aFarPlane)
		{
			constexpr float deg2rad = 0.0174532925;
			const float horizontalFovRadians = aFoV * deg2rad;

			const float verticalFovRadians
				= 2.f * std::atan(std::tan(horizontalFovRadians / 2.f) * (aResolution.y / aResolution.x));

			const float myXScale = 1.f / std::tanf(horizontalFovRadians * 0.5f);
			const float myYScale = 1.f / std::tanf(verticalFovRadians * 0.5f);

			const float Q = aFarPlane / (aFarPlane - aNearPlane);

			myValues[0][0] = myXScale;
			myValues[1][1] = myYScale;
			myValues[2][2] = Q;
			myValues[2][3] = 1.f;
			myValues[3][2] = -Q * aNearPlane;
			myValues[3][3] = 0.f;
		}

		const Vector3<T>& GetRight() const;
		const Vector3<T>& GetUp() const;
		const Vector3<T>& GetForward() const;
		const Vector3<T>& GetTranslation() const;

		T* GetPointer();
		const T* GetPointer() const;

	private:
		union
		{
			// Used for some good memcpy for performance reasons and initializations
			T myValues[4][4];

			struct
			{
				// Instead of keeping in array, i keep them in variables 
				// to avoid the confusion with 0 and 1 index
				// Also avoided the "my" prefix because it would look stoopid
				T m11, m12, m13, m14,
					m21, m22, m23, m24,
					m31, m32, m33, m34,
					m41, m42, m43, m44;
			};

			// Added for convenience, still unsure if should be public or private, private for now
			struct
			{
				Vector3<T> myX;
				float myXW;
				Vector3<T> myY;
				float myYW;
				Vector3<T> myZ;
				float myZW;
				Vector3<T> myW;
				float myWW;
			};

			struct
			{
				Vector4<T> myX4;
				Vector4<T> myY4;
				Vector4<T> myZ4;
				Vector4<T> myW4;
			};
		};
	};

	template <typename T>
	const Matrix4x4<T> Matrix4x4<T>::Zeroed = Matrix4x4<T>({ 0 });

	template <typename T>
	const Matrix4x4<T> Matrix4x4<T>::Identity = Matrix4x4<T>({
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
		});

	using Matrix4f = Matrix4x4<float>;
	using Matrix4d = Matrix4x4<double>;
}