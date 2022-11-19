#pragma once

#include "Common/CommonUtilities.h"

namespace Common
{
	template<typename T>
	class QuaternionT
	{
	public:
		QuaternionT<T>(); //Creates an Identity Quaternion
		QuaternionT<T>(const QuaternionT<T>& anOther); //Copy constructor
		QuaternionT<T>(const Vector3<T>& aEulerAngles); //Create a to-object-space quaternion from euler angles
		QuaternionT<T>(const Matrix4x4<T>& aRotationMatrix); //Create a rotation quaternion from a rotation matrix
		QuaternionT<T>(T aW, const Vector3<T>& aVector); //Set raw quaternion values. Not recommended unless you know quaternions very well.

		static QuaternionT<T> AxisAngle(CU::Vector3<T>& anAxis, T anAngle);
		void InitWithAxisAndRotation(const Vector3<T>& anAxis, T aRotation);
		void Normalize();
		QuaternionT<T> GetNormalized() const;

		QuaternionT<T> GetInverse() const;
		T GetMagnitude() const;
		QuaternionT<T> GetConjugate() const;
		T Dot(QuaternionT<T> anOther) const;

		Matrix4x4<T> ToMatrix() const;
		Vector3<T> EulerAngles() const;
		
		static float Angle(const QuaternionT<T>& aFirst, const QuaternionT<T>& aSecond);
		static QuaternionT<T> Slerp(const QuaternionT<T>& aFirst, QuaternionT<T> aSecond, T anInterpolation);
		static QuaternionT<T> SmoothSlerp(const QuaternionT<T>& aCurrentRotation, QuaternionT<T> aTargetRotation, T& aCurrentTime, T aTargetTime, T aDeltaTime = Time::DeltaTime);
		inline static QuaternionT<T> Identity() { return QuaternionT<T>(); };



	public:
		T myW;
		Vector3<T> myVector;
	};

	typedef QuaternionT<float> Quaternion;

	//Logically add two quaternions. If you are doing this, you are probably making your rotation quaternion invalid. Use multiplication to combine two rotations.
	/*template<typename T>
	QuaternionT<T> operator+(const QuaternionT<T>& aFirst, const QuaternionT<T>& aSecond); */

	//Combine two rotation quaternions into a single rotation. This must be done in reverse order to achieve the desired result.
	//In other words, multiplying a vector with quaternion q1 and then q2, is equivalent to multiplying with the single quaternion q2*q1.
	template <typename T>
	QuaternionT<T> operator*(const QuaternionT<T>& aFirst, const QuaternionT<T>& aSecond);

	template<typename T>
	QuaternionT<T> operator/(const QuaternionT<T>& aFirst, const QuaternionT<T>& aSecond);

	//Rotate a vector by the rotation defined by the quaternion
	template<typename T>
	Vector3<T> operator* (const Vector3<T>& aVector, const QuaternionT<T>& aQuaternion);

	template<typename T>
	QuaternionT<T> operator*(const QuaternionT<T>& aQuaternion, T aScalar);

	template<typename T>
	QuaternionT<T> operator/(const QuaternionT<T>& aQuaternion, T aScalar);

	template <typename T>
	bool operator!=(const QuaternionT<T>& aLeft, const QuaternionT<T>& aRight);
}