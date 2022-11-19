#include "pch.h"
#include "DebugDrawer.h"
#include <Engine/Renderer/Renderers/DebugRenderer.h>
#include "../Engine.h"
#include "Engine/Renderer/Culling/ViewFrustum.h"
#include "DebugMenu.h"

Engine::DebugDrawer::DebugDrawer()
{
}

void Engine::DebugDrawer::Update(const float aDeltaTime, DebugRenderer& aDebugRenderer)
{
#ifndef _RETAIL
	for (int i = static_cast<int>(myDrawEntries.size()) - 1; i >= 0; --i)
	{
		auto& drawEntry = myDrawEntries[i];

		// If they were 0, we draw them once only baby (kinda of a hack, but whatever)
		if (drawEntry.mySecondsRemaining < 0.f)
		{
			myDrawEntries[i] = myDrawEntries.back();
			myDrawEntries.pop_back();
		}
		else
		{
			drawEntry.myCallback(aDebugRenderer);
			drawEntry.mySecondsRemaining -= aDeltaTime;
		}
	}
#endif
}

void Engine::DebugDrawer::DrawLine2D(const DebugDrawFlags aFlag,
	const Vec2f& aFromNormalized,
	const Vec2f& aToNormalized,
	const float aDurationSeconds,
	const C::Vector4f& aColor /*= ourWhiteColor*/)
{
	AddDebugDrawEntry(
		aFlag,
		[=](DebugRenderer& aDebugRenderer)
		{
			aDebugRenderer.DrawLine2D(aFromNormalized, aToNormalized, aColor);
		},
		aDurationSeconds);
}

void Engine::DebugDrawer::DrawCircle2D(const DebugDrawFlags aFlag,
	const Vec2f& aPosNormalized,
	const float aRadiusRadians,
	const float aDurationSeconds,
	const C::Vector4f& aColor /*= ourWhiteColor*/)
{
	AddDebugDrawEntry(
		aFlag,
		[=](DebugRenderer& aDebugRenderer)
		{
			aDebugRenderer.DrawCircle2D(aPosNormalized, aRadiusRadians, aColor);
		},
		aDurationSeconds);
}

void Engine::DebugDrawer::DrawRectangle2D(const DebugDrawFlags aFlag,
	const Vec2f& aPosNormalized,
	const Vec2f& aHalfExtents,
	const float aDurationSeconds,
	const C::Vector4f& aColor /*= ourWhiteColor*/)
{
	AddDebugDrawEntry(
		aFlag,
		[=](DebugRenderer& aDebugRenderer)
		{
			aDebugRenderer.DrawRectangle2D(aPosNormalized, aHalfExtents, aColor);
		},
		aDurationSeconds);
}

void Engine::DebugDrawer::DrawLine3D(const DebugDrawFlags aFlag,
	const Vec3f& aFrom,
	const Vec3f& aTo,
	const float aDurationSeconds,
	const C::Vector4f& aColor /*= ourWhiteColor*/,
	const bool aDepthTested /*= true*/)
{
	AddDebugDrawEntry(
		aFlag,
		[=](DebugRenderer& aDebugRenderer)
		{
			aDebugRenderer.DrawLine3D(aFrom, aTo, aColor, aDepthTested);
		},
		aDurationSeconds);
}

void Engine::DebugDrawer::DrawCube3D(const DebugDrawFlags aFlag,
	const Vec3f& aCenterWorldPos,
	const Vec3f& aRotationRadians,
	const Vec3f& aSizeHalfExtents /*= ourOneVector*/,
	const float aDurationSeconds,
	const C::Vector4f& aColor /*= ourWhiteColor*/,
	const bool aDepthTested /*= true*/)
{
	AddDebugDrawEntry(
		aFlag,
		[=](DebugRenderer& aDebugRenderer)
		{
			aDebugRenderer.DrawCube3D(
				aCenterWorldPos, aRotationRadians, aSizeHalfExtents, aColor, aDepthTested);
		},
		aDurationSeconds);
}

void Engine::DebugDrawer::DrawSphere3D(const DebugDrawFlags aFlag,
	const Vec3f& aCenterWorldPos,
	const float aRadiusRadians /*= 1.f*/,
	const float aDurationSeconds,
	const C::Vector4f& aColor /*= ourWhiteColor*/,
	const bool aDepthTested /*= true*/)
{
	AddDebugDrawEntry(
		aFlag,
		[=](DebugRenderer& aDebugRenderer)
		{
			aDebugRenderer.DrawSphere3D(aCenterWorldPos, aRadiusRadians, aColor, aDepthTested);
		},
		aDurationSeconds);
}

void Engine::DebugDrawer::DrawCircle3D(const DebugDrawFlags aFlag,
	const Vec3f& aCenterWorldPos,
	const float aRadiusRadians,
	const float aDurationSeconds,
	const C::Vector4f& aColor,
	const bool aDepthTested)
{
	AddDebugDrawEntry(
		aFlag,
		[=](DebugRenderer& aDebugRenderer)
		{
			aDebugRenderer.DrawCircle3D(aCenterWorldPos, aRadiusRadians, aColor, aDepthTested);
		},
		aDurationSeconds);
}

void Engine::DebugDrawer::DrawFrustum(const DebugDrawFlags aFlag,
	const Vec3f& aPosition,
	const ViewFrustum& aViewFrustum,
	const float aDurationSeconds,
	const C::Vector4f& aColor,
	const bool aDepthTested)
{
	AddDebugDrawEntry(
		aFlag,
		[aPosition, &aViewFrustum, aColor, aDepthTested, this](
			DebugRenderer& aDebugRenderer)
		{
			aDebugRenderer.DrawFrustum(aPosition, aViewFrustum, aColor, aDepthTested);
		},
		aDurationSeconds);
}

void Engine::DebugDrawer::DrawFrustumOrthographic(const DebugDrawFlags aFlag,
	const Vec3f& aPosition,
	const ViewFrustum& aViewFrustum,
	const float aDurationSeconds,
	const C::Vector4f& aColor,
	const bool aDepthTested)
{
	AddDebugDrawEntry(
		aFlag,
		[aPosition, &aViewFrustum, aColor, aDepthTested, this](
			DebugRenderer& aDebugRenderer)
		{
			aDebugRenderer.DrawFrustumOrthographic(aPosition, aViewFrustum, aColor, aDepthTested);
		},
		aDurationSeconds);
}

void Engine::DebugDrawer::AddDebugDrawEntry(
	const DebugDrawFlags aFlag, const Callback aCallback, const float aDurationSeconds)
{
#ifndef _RETAIL
	if (!GetEngine().GetDebugDrawerSettings().IsDebugFlagSet(aFlag) && aFlag != DebugDrawFlags::Always)
	{
		return;
	}

	if (!GetEngine().GetDebugDrawerSettings().IsActive() && aFlag != DebugDrawFlags::Always)
	{
		return;
	}

	DrawEntry entry;
	{
		entry.myCallback = aCallback;
		entry.mySecondsRemaining = aDurationSeconds;
	}
	myDrawEntries.push_back(entry);
#endif
}
