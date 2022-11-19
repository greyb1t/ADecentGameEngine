#include "pch.h"
#include "FindAnimationClipsVisitor.h"
#include "Engine\Animation\NormalAnimation.h"
#include "Engine\Animation\Blending\BlendTree1D.h"
#include "Engine\ResourceManagement\Resources/AnimationClipResource.h"
#include "Engine\Animation\Blending\BlendTree2D.h"

Engine::FindAnimationClipsVisitor::FindAnimationClipsVisitor(
	std::vector<Engine::AnimationClip*>& aAllClips)
	: myAllClips(aAllClips)
{

}

void Engine::FindAnimationClipsVisitor::Visit(NormalAnimation& aNormalAnimation)
{
	const auto& clip = aNormalAnimation.myClip;

	if (clip && clip->IsValid())
	{
		myAllClips.push_back(&clip->Get());
	}
}

void Engine::FindAnimationClipsVisitor::Visit(BlendTree2D& aBlendTree)
{
	for (auto& node : aBlendTree.myNodes)
	{
		Playable* playable = node.GetPlayable();

		if (playable)
		{
			playable->Accept(*this);
		}
	}
}

void Engine::FindAnimationClipsVisitor::Visit(BlendTree1D& aBlendTree)
{
	for (auto& node : aBlendTree.myNodes)
	{
		Playable* playable = node.GetPlayable();

		if (playable)
		{
			playable->Accept(*this);
		}
	}
}
