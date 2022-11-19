#pragma once

#include "PlayableVisitor.h"

#include <vector>

namespace Engine
{
	class AnimationClip;
	class NormalAnimation;
	class BlendTree1D;
	class BlendTree2D;
}

namespace Engine
{
	class FindAnimationClipsVisitor : public PlayableVisitor
	{
	public:
		FindAnimationClipsVisitor(std::vector<AnimationClip*>& aAllClips);

		void Visit(NormalAnimation& aNormalAnimation) override;

		void Visit(BlendTree1D& aBlendTree) override;

		void Visit(BlendTree2D& aBlendTree) override;

	private:
		std::vector<AnimationClip*>& myAllClips;
	};
}