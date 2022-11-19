#pragma once
#include <Physics/Types.h>
namespace Engine {
	class Scene;
}

struct RayCastHit;
struct OverlapHit;

namespace ProjectileHelper
{
	/// <summary>
	/// 
	/// </summary>
	/// <returns>Return if hit STOP lm</returns>
	bool ScanProjectileArea(Engine::Scene* aScene, Vec3f aPosition, Vec3f aDirection, float aRange, float aRadius, LayerMask aStopLM, LayerMask aHitLM, std::vector<OverlapHit>& outHits, Vec3f outPosition);

	bool ScanProjectile(Engine::Scene* aScene, Vec3f aPosition, Vec3f aDirection, float aRange, float aRadius, LayerMask aStopLM, LayerMask aHitLM, int aRaysAmount, std::vector<RayCastHit>& outHits, Vec3f& outPosition);
};

