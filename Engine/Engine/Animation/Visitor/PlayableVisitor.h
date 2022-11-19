#pragma once

namespace Engine
{
	class NormalAnimation;
	class BlendTree1D;
	class BlendTree2D;

	class PlayableVisitor
	{
	public:
		virtual void Visit(NormalAnimation& aNormalAnimation) = 0;
		virtual void Visit(BlendTree1D& aBlendTree) = 0;
		virtual void Visit(BlendTree2D& aBlendTree) = 0;
	};
}