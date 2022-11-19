#pragma once

namespace Engine
{
	struct OurNode;
	class AnimationClip;
	struct Track;

	class AnimationClipLoader
	{
	public:
		bool Init();

		Owned<AnimationClip> LoadAnimationClip(const std::string& aPath);
	};
}