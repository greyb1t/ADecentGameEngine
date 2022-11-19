#pragma once
#include "Physics/Shape.h"

#define P_STATIC_COLOR CU::Vector4f(230 / 255.f, 255.f / 255.f, 230 / 255.f, 1.f)
#define P_STATIC_TRIGGER_COLOR CU::Vector4f(200.f / 255.f, 240.f / 255.f, 230 / 255.f, 1.f)
#define P_RIGID_COLOR CU::Vector4f(55.f / 255.f, 255.f / 255.f, 55 / 255.f, 1.f)
#define P_KINEMATIC_COLOR CU::Vector4f(0.f / 255.f, 170.f / 255.f, 0.f / 255.f, 1.f)
#define P_TRIGGER_COLOR CU::Vector4f(0 / 255.f, 255.f / 255.f, 125 / 255.f, 1.f)
#define P_CCT_COLOR CU::Vector4f(1.f, 165.f / 255.f, 0, 1)

namespace PhysicsDebugDrawer
{
	void DebugDrawShape(const Shape& aShape, const CU::Vector3f& aPosition, const CU::Quaternion& aRotation, const CU::Vector4f& aColor = { 1, 1, 1, 1});
};

