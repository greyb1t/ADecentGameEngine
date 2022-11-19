#include "pch.h"
#include "CameraRig.h"

#include "Engine/Shortcuts.h"
#include "Engine/DebugManager/DebugDrawer.h"

CameraRig::CameraRig()
{
}

CameraRig::~CameraRig()
{
}

void CameraRig::LoadMetrics(const CameraMetrics& someMetrics)
{
	myMetrics = someMetrics;
}

const Vec3f CameraRig::GetPositionOnRig(float aYaw, float aPitch, Vec3f aPlayerPosition) const
{
	Vec3f pivotPosition = aPlayerPosition + myMetrics.myPivotOffset;
	Vec3f upperCirclePos = pivotPosition + Vec3f(0.0f, myMetrics.myUpperCircleHeight, 0.0f);
	Vec3f lowerCirclePos = pivotPosition + Vec3f(0.0f, myMetrics.myLowerCircleHeight, 0.0f);

	Vec3f upperYawedPos = GetPointOnCircle(myMetrics.myUpperCirlceRadius, upperCirclePos, aYaw);
	Vec3f middleYawedPos = GetPointOnCircle(myMetrics.myMiddleCirlceRadius, pivotPosition, aYaw);
	Vec3f lowerYawedPos = GetPointOnCircle(myMetrics.myLowerCirlceRadius, lowerCirclePos, aYaw);


	//GDebugDrawer->DrawSphere3D(DebugDrawFlags::Gameplay, upperYawedPos, 5.0f);
	//GDebugDrawer->DrawSphere3D(DebugDrawFlags::Gameplay, middleYawedPos, 5.0f);
	//GDebugDrawer->DrawSphere3D(DebugDrawFlags::Gameplay, lowerYawedPos, 5.0f);

	//GDebugDrawer->DrawSphere3D(DebugDrawFlags::Gameplay, result, 10.0f, 0.0f, {1.0f, 0.0f, 0.0f, 1.0f});


	float lerpVal = (-aPitch + 1.0f) * 0.5f;

	Vec3f upperLerp = Math::Lerp(upperYawedPos, middleYawedPos, lerpVal);
	Vec3f lowerLerp = Math::Lerp(middleYawedPos, lowerYawedPos, lerpVal);
	return Math::Lerp(upperLerp, lowerLerp, lerpVal);
}

const Vec3f CameraRig::GetPivotPosition(Vec3f aPlayerPosition) const
{
	return aPlayerPosition + myMetrics.myPivotOffset;
}

void CameraRig::DrawRig(Vec3f aPlayerPosition)
{
	Vec3f playerPosition = aPlayerPosition + myMetrics.myPivotOffset;
	Vec3f upperCirclePos = playerPosition + Vec3f(0.0f, myMetrics.myUpperCircleHeight, 0.0f);
	Vec3f lowerCirclePos = playerPosition + Vec3f(0.0f, myMetrics.myLowerCircleHeight, 0.0f);

	//Debug
	GDebugDrawer->DrawCircle3D(DebugDrawFlags::UI, upperCirclePos, myMetrics.myUpperCirlceRadius);
	GDebugDrawer->DrawCircle3D(DebugDrawFlags::UI, playerPosition, myMetrics.myMiddleCirlceRadius);
	GDebugDrawer->DrawCircle3D(DebugDrawFlags::UI, lowerCirclePos, myMetrics.myLowerCirlceRadius);
}

const Vec3f CameraRig::GetPointOnCircle(float aRadius, Vec3f aCirclePos, float aRotation) const
{
	Vec3f circlePos;
	circlePos.x = aRadius * cosf(aRotation);
	circlePos.z = aRadius * sinf(aRotation);
	circlePos += aCirclePos;

	return circlePos;
}
