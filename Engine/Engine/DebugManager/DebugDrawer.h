#pragma once
#include "DebugDrawerSettings.h"

namespace Engine
{
class DebugRenderer;
class ViewFrustum;
}

namespace Engine
{
class DebugDrawer
{
	//using Callback = void (*)(Renderer::DebugRenderer&);
	using Callback = std::function<void(DebugRenderer&)>;

	struct DrawEntry
	{
		Callback myCallback;
		float mySecondsRemaining = 0.f;
	};

public:
	DebugDrawer();

	void Update(const float aDeltaTime, DebugRenderer& aDebugRenderer);

	void DrawLine2D(const DebugDrawFlags aFlag,
		const Vec2f& aFromNormalized,
		const Vec2f& aToNormalized,
		const float aDurationSeconds = 0.f,
		const C::Vector4f& aColor = ourWhiteColor);

	void DrawCircle2D(const DebugDrawFlags aFlag,
		const Vec2f& aPosNormalized,
		const float aRadiusRadians,
		const float aDurationSeconds = 0.f,
		const C::Vector4f& aColor = ourWhiteColor);

	void DrawRectangle2D(const DebugDrawFlags aFlag,
		const Vec2f& aPosNormalized,
		const Vec2f& aHalfExtents,
		const float aDurationSeconds = 0.f,
		const C::Vector4f& aColor = ourWhiteColor);

	void DrawLine3D(const DebugDrawFlags aFlag,
		const Vec3f& aFrom,
		const Vec3f& aTo,
		const float aDurationSeconds = 0.f,
		const C::Vector4f& aColor = ourWhiteColor,
		const bool aDepthTested = true);

	void DrawCube3D(const DebugDrawFlags aFlag,
		const Vec3f& aCenterWorldPos,
		const Vec3f& aRotationRadians,
		const Vec3f& aSizeHalfExtents = ourOneVector,
		const float aDurationSeconds = 0.f,
		const C::Vector4f& aColor = ourWhiteColor,
		const bool aDepthTested = true);

	void DrawSphere3D(const DebugDrawFlags aFlag,
		const Vec3f& aCenterWorldPos,
		const float aRadiusRadians = 1.f,
		const float aDurationSeconds = 0.f,
		const C::Vector4f& aColor = ourWhiteColor,
		const bool aDepthTested = true);

	void DrawCircle3D(const DebugDrawFlags aFlag,
		const Vec3f& aCenterWorldPos,
		const float aRadiusRadians = 1.f,
		const float aDurationSeconds = 0.f,
		const C::Vector4f& aColor = ourWhiteColor,
		const bool aDepthTested = true);

	void DrawFrustum(const DebugDrawFlags aFlag,
		const Vec3f& aPosition,
		const ViewFrustum& aViewFrustum,
		const float aDurationSeconds = 0.f,
		const C::Vector4f& aColor = ourWhiteColor,
		const bool aDepthTested = true);

	void DrawFrustumOrthographic(const DebugDrawFlags aFlag,
		const Vec3f& aPosition,
		const ViewFrustum& aViewFrustum,
		const float aDurationSeconds = 0.f,
		const C::Vector4f& aColor = ourWhiteColor,
		const bool aDepthTested = true);

private:
	void AddDebugDrawEntry(
		const DebugDrawFlags aFlag, const Callback aCallback, const float aDurationSeconds);

private:
	std::vector<DrawEntry> myDrawEntries;

	inline const static Vec3f ourOneVector = Vec3f(1.f, 1.f, 1.f);
	inline const static C::Vector4f ourWhiteColor = C::Vector4f(1.f, 1.f, 1.f, 1.f);
};
}