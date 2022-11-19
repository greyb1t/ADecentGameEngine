#pragma once

#include "Common/PlaneVolume.hpp"

namespace Engine { class EnvironmentLightComponent; }

namespace Common
{
	template <typename T>
	class Sphere;
}

namespace Engine
{
	class Camera;
	class RTransform;

	enum class FrustumType
	{
		Perspective,
		Orthographic,

		None
	};

	class ViewFrustum
	{
	public:
		ViewFrustum() = default;
		virtual ~ViewFrustum() = default;

		void InitOrthographic(const Camera& aCamera);

		void InitOrthographic(const Mat4f& aView, const Mat4f& aProjection);
		void InitOrthographic(EnvironmentLightComponent* aLight, const int aCascadeIndex);

		void InitPerspective(
			const RTransform& aTransformMatrix,
			const float aFovDegrees,
			const float aNearPlane,
			const float aFarPlane,
			const Vec2f& aResolution);

		void InitPerspective(const Camera& aCamera);

		bool IsSphereInside(const Common::Sphere<float>& aSphere) const;

		bool GetIsFrozen() const { return myIsFrozen; }
		void SetIsFrozen(bool val) { myIsFrozen = val; }

		Vec3f GetLastPos() const { return mylastpos; }

	protected:
		friend class DebugRenderer;

		FrustumType myFrustumType = FrustumType::None;

		Common::PlaneVolume<float> myPlanes;

		Vec3f nc; // myNearPlaneCenter
		Vec3f fc; // myFarPlaneCenter

		Vec3f ftl;
		Vec3f fbl;
		Vec3f fbr;
		Vec3f ftr;
		Vec3f ntl;
		Vec3f nbl;
		Vec3f nbr;
		Vec3f ntr;

		bool myIsFrozen = false;
		Vec3f mylastpos;
	};
}