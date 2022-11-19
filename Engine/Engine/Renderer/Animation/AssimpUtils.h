#pragma once

namespace Engine
{
	Mat4f AiMatrixToCUMatrix(aiMatrix4x4 aMatrix);
	aiMatrix4x4 CUMatrixToAiMatrix(Mat4f aMatrix);

	Quatf AiQuaternionToCUQuaternion(const aiQuaternion& aQuat);

	Vec3f AiVector3DToCUVector3f(const aiVector3D& aiVector);
}