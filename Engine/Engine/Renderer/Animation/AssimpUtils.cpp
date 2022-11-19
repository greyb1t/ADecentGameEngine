#include "pch.h"
#include "AssimpUtils.h"

namespace Engine
{
	Mat4f AiMatrixToCUMatrix(aiMatrix4x4 aMatrix)
	{
		Mat4f result;

		result(1, 1) = aMatrix.a1;
		result(1, 2) = aMatrix.a2;
		result(1, 3) = aMatrix.a3;
		result(1, 4) = aMatrix.a4;

		result(2, 1) = aMatrix.b1;
		result(2, 2) = aMatrix.b2;
		result(2, 3) = aMatrix.b3;
		result(2, 4) = aMatrix.b4;

		result(3, 1) = aMatrix.c1;
		result(3, 2) = aMatrix.c2;
		result(3, 3) = aMatrix.c3;
		result(3, 4) = aMatrix.c4;

		result(4, 1) = aMatrix.d1;
		result(4, 2) = aMatrix.d2;
		result(4, 3) = aMatrix.d3;
		result(4, 4) = aMatrix.d4;

		return result;
	}

	aiMatrix4x4 CUMatrixToAiMatrix(Mat4f aMatrix)
	{
		aiMatrix4x4 result;

		result.a1 = aMatrix(1, 1);
		result.a2 = aMatrix(1, 2);
		result.a3 = aMatrix(1, 3);
		result.a4 = aMatrix(1, 4);

		result.b1 = aMatrix(2, 1);
		result.b2 = aMatrix(2, 2);
		result.b3 = aMatrix(2, 3);
		result.b4 = aMatrix(2, 4);

		result.c1 = aMatrix(3, 1);
		result.c2 = aMatrix(3, 2);
		result.c3 = aMatrix(3, 3);
		result.c4 = aMatrix(3, 4);

		result.d1 = aMatrix(4, 1);
		result.d2 = aMatrix(4, 2);
		result.d3 = aMatrix(4, 3);
		result.d4 = aMatrix(4, 4);

		return result;
	}

	Quatf AiQuaternionToCUQuaternion(const aiQuaternion& aQuat)
	{
		return Quatf(aQuat.w, Vec3f(aQuat.x, aQuat.y, aQuat.z));
	}

	Vec3f AiVector3DToCUVector3f(const aiVector3D& aiVector)
	{
		return Vec3f(aiVector.x, aiVector.y, aiVector.z);
	}
}