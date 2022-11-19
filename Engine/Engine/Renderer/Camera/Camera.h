#pragma once

#include "../RTransform.h"
#include "../Utils/Observer.h"
#include "Engine/Renderer/Culling/ViewFrustum.h"

namespace Engine
{
	class CameraComponentNew;
	class Reflector;
}

namespace Engine
{
	class WindowHandler;
	class ViewFrustum;

	enum class CameraType
	{
		Perspective,
		Orthographic,

		None
	};

	class Camera : public Observer
	{
	public:
		Camera();
		virtual ~Camera();

		void InitPerspective(
			const float aHorizontalFovDegrees,
			const Vec2f& aResolution,
			const float aNearPlane,
			const float aFarPlane);

		void InitOrthographic(
			const Vec2f& aResolution,
			const float aNearPlane,
			const float aFarPlane);

		void UpdateProjectionMatrix();

		void SetFov(const float aDegres);

		const Mat4f& GetProjectionMatrix() const;

		const RTransform& GetTransform() const;

		void SetRotationEuler(const Vec3f& aRotation);
		void SetRotationQuaternion(const Quatf& aRotation);
		void SetPosition(const Vec3f& aPosition);
		void SetTransform(const RTransform& aTransform);

		const RTransform& GetOffsetTransform() const;

		// Only added to support camera shake, otherwise we'd required parent child relationships
		void SetOffsetRotationEuler(const Vec3f& aRotation);
		void SetOffsetRotationQuaternion(const Quatf& aRotation);
		void SetOffsetPosition(const Vec3f& aPosition);
		void SetOffsetTransform(const RTransform& aTransform);

		float GetNearPlane() const;
		float GetFarPlane() const;
		float GetFovDegrees() const;

		Vec2f GetResolution() const;

		const ViewFrustum& GetViewFrustum() const;
		ViewFrustum& GetViewFrustum();

		void Receive(const EventType aEventType, const std::any& aValue) override;

		void Reflect(Reflector& aReflector, CameraComponentNew& aCameraComponent);

	protected:
		float myNearPlane = 50.f;
		float myFarPlane = 25000.f;

		// TODO: move this to perspetive..
		float myFovDegrees = 90.f;

		Mat4f myProjectionMatrix;

		mutable RTransform myTransformCombined;

		RTransform myTransform;
		RTransform myOffsetTransform;

		Vec2f myResolution;

		mutable ViewFrustum myViewFrustum;
		mutable bool myIsViewFrustumDirty = false;

		CameraType myCameraType = CameraType::None;
	};
}
