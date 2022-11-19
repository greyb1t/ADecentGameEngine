#include "pch.h"
#include "AnimationClipLoader.h"
#include "..\Animation\AnimationClip.h"

bool Engine::AnimationClipLoader::Init()
{
	return true;
}

Owned<Engine::AnimationClip> Engine::AnimationClipLoader::LoadAnimationClip(const std::string& aPath)
{
	auto animClip = MakeOwned<AnimationClip>();

	if (!animClip->Init(aPath))
	{
		LOG_ERROR(LogType::Renderer) << "Failed to load animation clip " << aPath;
		return nullptr;
	}

	return std::move(animClip);
}
