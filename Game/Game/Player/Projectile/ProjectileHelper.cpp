#include "pch.h"
#include "ProjectileHelper.h"
#include <Engine/Scene/Scene.h>

bool ProjectileHelper::ScanProjectileArea(Engine::Scene* aScene, Vec3f aPosition, Vec3f aDirection, float aRange, float aRadius, LayerMask aStopLM, LayerMask aHitLM, std::vector<OverlapHit>& outHits, Vec3f outPosition)
{
	bool result = true;

	RayCastHit hit;
	if (aScene->RayCast(aPosition, aDirection, aRange, aStopLM, hit)) 
	{
		aRange = hit.Distance;
		result = false;
	}

	physx::PxCapsuleGeometry geo(aRadius, aRange * 0.5f);

	Quatf rot = Quatf(Mat4f::LookAt({ 0,0,0 }, aDirection));
	aScene->GeometryCastAll(geo, aPosition + aDirection * aRange * 0.5f, rot, aHitLM, outHits);

	outPosition = aPosition + aDirection * aRange;
	return result;
}

bool ProjectileHelper::ScanProjectile(Engine::Scene* aScene, Vec3f aPosition, Vec3f aDirection, float aRange, float aRadius, LayerMask aStopLM, LayerMask aHitLM, int aRaysAmount, std::vector<RayCastHit>& outHits, Vec3f& outPosition)
{
	bool result = true;

	RayCastHit hit;
	if (aScene->RayCast(aPosition, aDirection, aRange, aStopLM, hit))
	{
		aRange = hit.Distance;
		result = false;
	}
	outPosition = aPosition + aDirection * aRange;
	aScene->RayCastAll(aPosition, aDirection, aRange, aHitLM, outHits);

	Mat4f matrix = Mat4f::LookAt({ 0,0,0 }, aDirection);

	const float rayDegree = (360.f / aRaysAmount) * Math::Deg2Rad;

	for (int i = 0; i < aRaysAmount; i++) 
	{
		Quatf q = Quatf::AxisAngle(aDirection, rayDegree * i);


		//Mat4f m = matrix * Mat4f::CreateRotationAroundZ(rayDegree * i);

		Vec3f offset = { 1, 0, 0 };
		Quatf rotation = Quatf(Vec3f(0,0,rayDegree * i));

		Mat4f m = q.ToMatrix();

		Vec3f offsetPosition = (offset * rotation * Quatf(matrix)) * aRadius;
		aScene->RayCastAll(aPosition + offsetPosition, aDirection, aRange, aHitLM, outHits);

		GDebugDrawer->DrawLine3D(DebugDrawFlags::Player, aPosition + offsetPosition, aPosition + offsetPosition + aDirection * aRange, 20.f, Vec4f(0, 0, 1.f, 1.f));

	}
	return result;
}
