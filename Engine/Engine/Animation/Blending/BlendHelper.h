#pragma once

#include "Engine/Animation/AnimationTransformResult.h"

namespace Engine
{
	class BlendMask;
}

namespace BlendHelper
{
	Engine::AnimationTransformResult Blend(
		const Engine::AnimationTransformResult& aResult1,
		const Engine::AnimationTransformResult& aResult2,
		const float aBlendPercent);

	Engine::AnimationTransformResult Blend(
		const Engine::AnimationTransformResult& aResult1,
		const Engine::AnimationTransformResult& aResult2,
		const Engine::AnimationTransformResult& aResult3,
		const Engine::AnimationTransformResult& aBindPose,
		const float aU, 
		const float aV, 
		const float aW);

	Engine::AnimationTransformResult BlendAdditive(
		const Engine::AnimationTransformResult& aResult1,
		const Engine::AnimationTransformResult& aResult2,
		const Engine::AnimationTransformResult& aBindPose,
		const float aBlendPercent);

	Engine::AnimationTransformResult BlendMasked(
		const Engine::AnimationTransformResult& aResult1,
		const Engine::AnimationTransformResult& aResult2,
		const float aBlendPercent,
		const Engine::BlendMask& aMask);

	Engine::AnimationTransformResult BlendAdditiveMasked(
		const Engine::AnimationTransformResult& aResult1,
		const Engine::AnimationTransformResult& aResult2,
		const Engine::AnimationTransformResult& aBindPose,
		const float aBlendPercent,
		const Engine::BlendMask& aMask);
}
