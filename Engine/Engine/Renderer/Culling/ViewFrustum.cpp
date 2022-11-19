#include "pch.h"
#include "ViewFrustum.h"
#include "../Camera/Camera.h"
#include "../Shortcuts.h"
#include "../DebugManager/DebugDrawerSettings.h"
#include "../DebugManager/DebugDrawer.h"
#include "../GameObject/Components/EnvironmentLightComponent.h"
#include "../GameObject/GameObject.h"

namespace Engine
{
	void ViewFrustum::InitOrthographic(const Camera& aCamera)
	{
		if (myIsFrozen)
		{
			return;
		}

		// 1st letter:
		// f = far plane
		// n = near plane

		// 2nd letter:
		// t = top plane
		// b = bottom plane

		// 3rd letter:
		// l = left plane
		// r = right plane

		// http://www.lighthouse3d.com/tutorials/view-frustum-culling/geometric-approach-extracting-the-planes/

		const RTransform& camTransform = aCamera.GetTransform();
		const Mat4f& camTransformMatrix = camTransform.ToMatrix();

		const auto Z = camTransformMatrix.GetForward().GetNormalized() * -1.f;
		const auto& Y = camTransformMatrix.GetUp().GetNormalized();
		const auto& X = camTransformMatrix.GetRight().GetNormalized();

		const auto& p = camTransform.GetPosition();
		mylastpos = p;

		const float farD = aCamera.GetFarPlane();
		const float nearD = aCamera.GetNearPlane();

		const auto& resolution = aCamera.GetResolution();
		const float ratio = /*resolution.x / resolution.y*/ 1.f;

		// TODO: I don't know if should be / 2.f or not!
		const float nh = resolution.x / 2.f;
		const float nw = resolution.y / 2.f;

		const float fh = resolution.x / 2.f;
		const float fw = resolution.y / 2.f;

		nc = p - Z * nearD;
		fc = p - Z * farD;

		ntl = nc + (Y * nh) - (X * nw);
		ntr = nc + (Y * nh) + (X * nw);
		nbl = nc - (Y * nh) - (X * nw);
		nbr = nc - (Y * nh) + (X * nw);

		// Calculate the corners of the near and far plane
		ftl = fc + (Y * fh) - (X * fw);
		ftr = fc + (Y * fh) + (X * fw);
		fbl = fc - (Y * fh) - (X * fw);
		fbr = fc - (Y * fh) + (X * fw);

		myPlanes = { };

		// Create the planes based on the corners
		// Clockwise order of the points due to being in left handed coordinate system

		// Far plane
		myPlanes.AddPlane(C::Plane<float>(ftl, ftr, fbr));

		// Near plane
		myPlanes.AddPlane(C::Plane<float>(nbr, ntr, ntl));

		// Left plane
		myPlanes.AddPlane(C::Plane<float>(ntl, ftl, fbl));

		// Right plane
		myPlanes.AddPlane(C::Plane<float>(fbr, ftr, ntr));

		// Top plane
		myPlanes.AddPlane(C::Plane<float>(ntl, ntr, ftr));

		// Bottom
		myPlanes.AddPlane(C::Plane<float>(nbr, nbl, fbl));
	}

	void ViewFrustum::InitOrthographic(const Mat4f& aView, const Mat4f& aProjection)
	{
		if (myIsFrozen)
			return;

		Mat4f inverseProjection = aProjection.Inverse();
		inverseProjection = inverseProjection * aView;

		Vec4f ntl = Vec4f(-1, 1, 0, 1) * inverseProjection;
		Vec4f ntr = Vec4f(1, 1, 0, 1) * inverseProjection;
		Vec4f nbl = Vec4f(-1, -1, 0, 1) * inverseProjection;
		Vec4f nbr = Vec4f(1, -1, 0, 1) * inverseProjection;

		Vec4f ftl = Vec4f(-1, 1, 1, 1) * inverseProjection;
		Vec4f ftr = Vec4f(1, 1, 1, 1) * inverseProjection;
		Vec4f fbl = Vec4f(-1, -1, 1, 1) * inverseProjection;
		Vec4f fbr = Vec4f(1, -1, 1, 1) * inverseProjection;

		ntl /= ntl.w;
		ntr /= ntr.w;
		nbl /= nbl.w;
		nbr /= nbr.w;

		ftl /= ftl.w;
		ftr /= ftr.w;
		fbl /= fbl.w;
		fbr /= fbr.w;

		//GDebugDrawer->DrawSphere3D(DebugDrawFlags::Always, ntl.ToVec3(), 100.f, 0.0f, Vec4f(0, 0, 1, 1));
		//GDebugDrawer->DrawSphere3D(DebugDrawFlags::Always, ntr.ToVec3(), 100.f, 0.0f, Vec4f(0, 0, 1, 1));
		//GDebugDrawer->DrawSphere3D(DebugDrawFlags::Always, nbl.ToVec3(), 100.f, 0.0f, Vec4f(0, 0, 1, 1));
		//GDebugDrawer->DrawSphere3D(DebugDrawFlags::Always, nbr.ToVec3(), 100.f, 0.0f, Vec4f(0, 0, 1, 1));
		//GDebugDrawer->DrawSphere3D(DebugDrawFlags::Always, ftl.ToVec3(), 100.f, 0.0f, Vec4f(0, 0, 1, 1));
		//GDebugDrawer->DrawSphere3D(DebugDrawFlags::Always, ftr.ToVec3(), 100.f, 0.0f, Vec4f(0, 0, 1, 1));
		//GDebugDrawer->DrawSphere3D(DebugDrawFlags::Always, fbl.ToVec3(), 100.f, 0.0f, Vec4f(0, 0, 1, 1));
		//GDebugDrawer->DrawSphere3D(DebugDrawFlags::Always, fbr.ToVec3(), 100.f, 0.0f, Vec4f(0, 0, 1, 1));
		//
		//GDebugDrawer->DrawLine3D(DebugDrawFlags::Always, ntl.ToVec3(), ftl.ToVec3(), 0.0f, Vec4f(0, 1, 0, 1));
		//GDebugDrawer->DrawLine3D(DebugDrawFlags::Always, ntr.ToVec3(), ftr.ToVec3(), 0.0f, Vec4f(0, 1, 0, 1));
		//GDebugDrawer->DrawLine3D(DebugDrawFlags::Always, nbl.ToVec3(), fbl.ToVec3(), 0.0f, Vec4f(0, 1, 0, 1));
		//GDebugDrawer->DrawLine3D(DebugDrawFlags::Always, nbr.ToVec3(), fbr.ToVec3(), 0.0f, Vec4f(0, 1, 0, 1));

		myPlanes = {};
		// Far plane
		myPlanes.AddPlane(C::Plane<float>(ftl.ToVec3(), ftr.ToVec3(), fbr.ToVec3()));

		// Near plane
		myPlanes.AddPlane(C::Plane<float>(nbr.ToVec3(), ntr.ToVec3(), ntl.ToVec3()));

		// Left plane
		myPlanes.AddPlane(C::Plane<float>(ntl.ToVec3(), ftl.ToVec3(), fbl.ToVec3()));

		// Right plane
		myPlanes.AddPlane(C::Plane<float>(fbr.ToVec3(), ftr.ToVec3(), ntr.ToVec3()));

		// Top plane
		myPlanes.AddPlane(C::Plane<float>(ntl.ToVec3(), ntr.ToVec3(), ftr.ToVec3()));

		// Bottom
		myPlanes.AddPlane(C::Plane<float>(nbr.ToVec3(), nbl.ToVec3(), fbl.ToVec3()));
	}

	void ViewFrustum::InitOrthographic(EnvironmentLightComponent* aLight, const int aCascadeIndex)
	{
		const auto& cascadeData = aLight->GetCascadeData(aCascadeIndex);
		const Mat4f& cascadeView = aLight->GetCascadeView(aCascadeIndex);

		const Vec3f camPos = cascadeView.GetTranslation();
		const Vec3f camForward = cascadeView.GetForward();
		const Vec3f camUp = cascadeView.GetUp();
		const Vec3f camRight = cascadeView.GetRight();

		const float halfWidth = cascadeData.resolution.x * 0.5f;
		const float halfHeight = cascadeData.resolution.y * 0.5f;

		Vec3f fc = camPos + camForward * cascadeData.farPlane;
		Vec3f ftl = fc + (camUp * halfHeight) + (camRight * halfWidth);
		Vec3f ftr = fc + (camUp * halfHeight) - (camRight * halfWidth);
		Vec3f fbl = fc - (camUp * halfHeight) - (camRight * halfWidth);
		Vec3f fbr = fc - (camUp * halfHeight) + (camRight * halfWidth);

		Vec3f nc = camPos + camForward * cascadeData.nearPlane;
		Vec3f ntl = nc + (camUp * halfHeight) + (camRight * halfWidth);
		Vec3f ntr = nc + (camUp * halfHeight) - (camRight * halfWidth);
		Vec3f nbl = nc - (camUp * halfHeight) - (camRight * halfWidth);
		Vec3f nbr = nc - (camUp * halfHeight) + (camRight * halfWidth);

		//GDebugDrawer->DrawSphere3D(DebugDrawFlags::Always, ntl, 100.f, 0.0f, Vec4f(0, 0, 1, 1));
		//GDebugDrawer->DrawSphere3D(DebugDrawFlags::Always, ntr, 100.f, 0.0f, Vec4f(0, 0, 1, 1));
		//GDebugDrawer->DrawSphere3D(DebugDrawFlags::Always, nbl, 100.f, 0.0f, Vec4f(0, 0, 1, 1));
		//GDebugDrawer->DrawSphere3D(DebugDrawFlags::Always, nbr, 100.f, 0.0f, Vec4f(0, 0, 1, 1));
		//GDebugDrawer->DrawSphere3D(DebugDrawFlags::Always, ftl, 100.f, 0.0f, Vec4f(0, 0, 1, 1));
		//GDebugDrawer->DrawSphere3D(DebugDrawFlags::Always, ftr, 100.f, 0.0f, Vec4f(0, 0, 1, 1));
		//GDebugDrawer->DrawSphere3D(DebugDrawFlags::Always, fbl, 100.f, 0.0f, Vec4f(0, 0, 1, 1));
		//GDebugDrawer->DrawSphere3D(DebugDrawFlags::Always, fbr, 100.f, 0.0f, Vec4f(0, 0, 1, 1));
		//
		//GDebugDrawer->DrawLine3D(DebugDrawFlags::Always, ntl, ftl, 0.0f, Vec4f(0, 1, 0, 1));
		//GDebugDrawer->DrawLine3D(DebugDrawFlags::Always, ntr, ftr, 0.0f, Vec4f(0, 1, 0, 1));
		//GDebugDrawer->DrawLine3D(DebugDrawFlags::Always, nbl, fbl, 0.0f, Vec4f(0, 1, 0, 1));
		//GDebugDrawer->DrawLine3D(DebugDrawFlags::Always, nbr, fbr, 0.0f, Vec4f(0, 1, 0, 1));
	}

	void ViewFrustum::InitPerspective(const RTransform& aTransformMatrix, const float aFovDegrees,
		const float aNearPlane, const float aFarPlane, const Vec2f& aResolution)
	{
		if (myIsFrozen)
		{
			return;
		}

		// 1st letter:
		// f = far plane
		// n = near plane

		// 2nd letter:
		// t = top plane
		// b = bottom plane

		// 3rd letter:
		// l = left plane
		// r = right plane

		// http://www.lighthouse3d.com/tutorials/view-frustum-culling/geometric-approach-extracting-the-planes/

		const RTransform& camTransform = aTransformMatrix;
		const Mat4f& camTransformMatrix = camTransform.ToMatrix();

		const auto Z = camTransformMatrix.GetForward().GetNormalized() * -1.f;
		const auto& Y = camTransformMatrix.GetUp().GetNormalized();
		const auto& X = camTransformMatrix.GetRight().GetNormalized();

		const float fovRadians = (Math::PI / 180.f) * aFovDegrees * 0.5f;

		const float verticalFovRadians = 2.f * std::atan(std::tan(fovRadians) * (aResolution.y / aResolution.x));

		const float tang = tan(verticalFovRadians * 0.5f);

		const auto& p = camTransform.GetPosition();
		mylastpos = p;

		const float farD = aFarPlane;
		const float nearD = aNearPlane;

		const float ratio = aResolution.x / aResolution.y;

		// hmm..
		const float nh = tang * nearD;
		const float nw = nh * ratio;

		const float fh = tang * farD;
		const float fw = fh * ratio;

		nc = p - Z * nearD;
		fc = p - Z * farD;

		ntl = nc + (Y * nh) - (X * nw);
		ntr = nc + (Y * nh) + (X * nw);
		nbl = nc - (Y * nh) - (X * nw);
		nbr = nc - (Y * nh) + (X * nw);

		// Calculate the corners of the near and far plane
		ftl = fc + (Y * fh) - (X * fw);
		ftr = fc + (Y * fh) + (X * fw);
		fbl = fc - (Y * fh) - (X * fw);
		fbr = fc - (Y * fh) + (X * fw);

		myPlanes = { };

		// Create the planes based on the corners
		// Clockwise order of the points due to being in left handed coordinate system

		// Far plane
		myPlanes.AddPlane(C::Plane<float>(ftl, ftr, fbr));

		// Near plane
		myPlanes.AddPlane(C::Plane<float>(nbr, ntr, ntl));

		// Left plane
		myPlanes.AddPlane(C::Plane<float>(ntl, ftl, fbl));

		// Right plane
		myPlanes.AddPlane(C::Plane<float>(fbr, ftr, ntr));

		// Top plane
		myPlanes.AddPlane(C::Plane<float>(ntl, ntr, ftr));

		// Bottom
		myPlanes.AddPlane(C::Plane<float>(nbr, nbl, fbl));
	}

	void ViewFrustum::InitPerspective(const Camera& aCamera)
	{
		if (myIsFrozen)
		{
			return;
		}

		const auto& resolution = aCamera.GetResolution();

		InitPerspective(aCamera.GetTransform(),
		     aCamera.GetFovDegrees(),
		     aCamera.GetNearPlane(),
		     aCamera.GetFarPlane(),
		     resolution);
	}

	bool ViewFrustum::IsSphereInside(const C::Sphere<float>& aSphere) const
	{
		return myPlanes.IsInside(aSphere);
	}
}