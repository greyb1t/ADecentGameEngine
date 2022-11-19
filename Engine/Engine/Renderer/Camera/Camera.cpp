#include "pch.h"
#include "Camera.h"
#include "Engine/Renderer/Culling/ViewFrustum.h"
#include "Engine/GameObject/Components/CameraComponent.h"
#include "Engine/Reflection/Reflector.h"
#include "Engine/Renderer/WindowHandler.h"

namespace Engine
{
Camera::Camera()
{
	// NOTE: DO NOT USE BELOW 0.01
	// A near plane value too low will causes visual issues on models
	myNearPlane = 1.f;
	myFarPlane = 25000.f;
}

Camera::~Camera()
{

}

void Camera::InitPerspective(const float aHorizontalFovDegrees,
	const Vec2f& aResolution,
	const float aNearPlane,
	const float aFarPlane)
{
	myCameraType = CameraType::Perspective;

	myFovDegrees = aHorizontalFovDegrees;
	myResolution = aResolution;

	myNearPlane = aNearPlane;
	myFarPlane = aFarPlane;

	UpdateProjectionMatrix();

	myViewFrustum.InitPerspective(*this);
}

void Camera::InitOrthographic(
	const Vec2f& aResolution, const float aNearPlane, const float aFarPlane)
{
	myCameraType = CameraType::Orthographic;

	myResolution = aResolution;

	myNearPlane = aNearPlane;
	myFarPlane = aFarPlane;

	myProjectionMatrix(1, 1) = 2.f / aResolution.x;
	myProjectionMatrix(2, 2) = 2.f / aResolution.y;
	myProjectionMatrix(3, 3) = 1.f / (myFarPlane - myNearPlane);
	myProjectionMatrix(4, 3) = myNearPlane / (myNearPlane - myFarPlane);
	myProjectionMatrix(4, 4) = 1.f;

	myViewFrustum.InitOrthographic(*this);
}

void Camera::UpdateProjectionMatrix()
{
	const float horizontalFovRadians = myFovDegrees * Math::DegToRad;

	const float verticalFovRadians
		= 2.f * std::atan(std::tan(horizontalFovRadians / 2.f) * (myResolution.y / myResolution.x));

	const float myXScale = 1.f / std::tanf(horizontalFovRadians * 0.5f);
	const float myYScale = 1.f / std::tanf(verticalFovRadians * 0.5f);

	const float Q = myFarPlane / (myFarPlane - myNearPlane);

	myProjectionMatrix(1, 1) = myXScale;
	myProjectionMatrix(2, 2) = myYScale;
	myProjectionMatrix(3, 3) = Q;
	myProjectionMatrix(3, 4) = 1.f;
	myProjectionMatrix(4, 3) = -Q * myNearPlane;
	myProjectionMatrix(4, 4) = 0.f;
}

void Camera::SetFov(const float aDegres)
{
	myFovDegrees = aDegres;

	UpdateProjectionMatrix();
}

const Mat4f& Camera::GetProjectionMatrix() const
{
	return myProjectionMatrix;
}

const RTransform& Camera::GetTransform() const
{
	myTransformCombined = myTransform.Combine(myOffsetTransform);

	return myTransformCombined;
}

void Camera::SetRotationEuler(const Vec3f& aRotation)
{
	myTransform.SetRotationEuler(aRotation);
	myIsViewFrustumDirty = true;
}

void Camera::SetRotationQuaternion(const Quatf& aRotation)
{
	myTransform.SetRotationQuaternion(aRotation);
	myIsViewFrustumDirty = true;
}

void Camera::SetPosition(const Vec3f& aPosition)
{
	myTransform.SetPosition(aPosition);
	myIsViewFrustumDirty = true;
}

void Camera::SetTransform(const RTransform& aTransform)
{
	myTransform = aTransform;
	myIsViewFrustumDirty = true;
}

const RTransform& Camera::GetOffsetTransform() const
{
	return myOffsetTransform;
}

void Camera::SetOffsetRotationEuler(const Vec3f& aRotation)
{
	myOffsetTransform.SetRotationEuler(aRotation);
	myIsViewFrustumDirty = true;
}

void Camera::SetOffsetRotationQuaternion(const Quatf& aRotation)
{
	myOffsetTransform.SetRotationQuaternion(aRotation);
	myIsViewFrustumDirty = true;
}

void Camera::SetOffsetPosition(const Vec3f& aPosition)
{
	myOffsetTransform.SetPosition(aPosition);
	myIsViewFrustumDirty = true;
}

void Camera::SetOffsetTransform(const RTransform& aTransform)
{
	myOffsetTransform = aTransform;
	myIsViewFrustumDirty = true;
}

float Camera::GetNearPlane() const
{
	return myNearPlane;
}

float Camera::GetFarPlane() const
{
	return myFarPlane;
}

float Camera::GetFovDegrees() const
{
	return myFovDegrees;
}

Vec2f Camera::GetResolution() const
{
	return myResolution;
}

const ViewFrustum& Camera::GetViewFrustum() const
{
	if (myIsViewFrustumDirty)
	{
		switch (myCameraType)
		{
			case CameraType::Perspective:
				myViewFrustum.InitPerspective(*this);
				break;
			case CameraType::Orthographic:
				myViewFrustum.InitOrthographic(*this);
				break;

			default:
				assert(false);
				break;
		};

		myIsViewFrustumDirty = false;
	}

	return myViewFrustum;
}

ViewFrustum& Camera::GetViewFrustum()
{
	return myViewFrustum;
}

void Camera::Receive(const EventType aEventType, const std::any& aValue)
{
	if (aEventType == EventType::WindowResized)
	{
		const auto resizeData = std::any_cast<WindowResizedData>(aValue);

		switch (myCameraType)
		{
			case CameraType::Perspective:
				InitPerspective(myFovDegrees, resizeData.myClientSize, myNearPlane, myFarPlane);
				break;
			case CameraType::Orthographic:
				InitOrthographic(resizeData.myClientSize, myNearPlane, myFarPlane);
				break;
			default:
				break;
		};
	}
}

void Camera::Reflect(Reflector& aReflector, CameraComponentNew& aCameraComponent)
{
	ReflectorResult result = ReflectorResult_None;

	result |= aReflector.Reflect(myNearPlane, "Near Plane");
	result |= aReflector.Reflect(myFarPlane, "Far Plane");

	result |= aReflector.Reflect(myFovDegrees, "FOV");

	result |= aReflector.Reflect(myResolution, "Resolution");

	int camType = static_cast<int>(myCameraType);
	result |= aReflector.Reflect(camType, "CameraType");
	myCameraType = static_cast<CameraType>(camType);

	if (result & (ReflectorResult_Changed | ReflectorResult_Changing))
	{
		aCameraComponent.InitCamera();
	}
}
}
