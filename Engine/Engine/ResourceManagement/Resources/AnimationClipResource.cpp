#include "pch.h"
#include "AnimationClipResource.h"
#include "../ResourceThreadContext.h"
#include "Engine/Renderer/Animation/AnimationClipLoader.h"
#include "Engine/Renderer/Animation/AnimationClip.h"

Engine::AnimationClipResource::AnimationClipResource(
	ResourceManager& aResourceManager,
	const std::string& aPath,
	const ResourceId& aId)
	: ResourceBase(aResourceManager, aPath, aId)
{
}

Engine::AnimationClipResource::~AnimationClipResource()
{
}

Engine::AnimationClipResource::AnimationClipResource(const AnimationClipResource& aOther) :
	ResourceBase(*aOther.myResourceManager, aOther.myPath, aOther.myIdentifier)
{
	assert(aOther.myAnimationClip == nullptr && "why is this not nullptr? it is "
		"supposed to be because we only ever copy this in ResourceManager "
		"before it has been fully created");
}

Engine::AnimationClip& Engine::AnimationClipResource::Get() const
{
	return *myAnimationClip;
}

bool Engine::AnimationClipResource::Create(ResourceThreadData& aThreadData)
{
	auto animationClip = aThreadData.GetAnimationClipLoader().LoadAnimationClip(myPath);

	if (animationClip == nullptr)
	{
		return false;
	}

	myAnimationClip = std::move(animationClip);

	return true;
}

void Engine::AnimationClipResource::Swap(ResourceBase* aResourceBase)
{
	// We know for a fact that aResourceBase input argument is of the same class
	// otherwise something went wrong in the ResourceManager
	auto other = reinterpret_cast<AnimationClipResource*>(aResourceBase);

	myAnimationClip.swap(other->myAnimationClip);
}

void Engine::AnimationClipResource::PostCreation(ResourceThreadData& aResourceThreadData)
{
	myResourceManager->GetFileWatcher()->WatchFile(myPath,
		[this, &aResourceThreadData](const std::string& aPath)
	{
		// this lambda will be called from render thread (main thread)
		// so its ok to edit stuff here because renderer 
		// won't touch the sprite during this time
		Create(aResourceThreadData);
	});
}

void Engine::AnimationClipResource::RequestMe() const
{
	// Sadly this is required to avoid having the ResourceBase::RequestLoading as a NON-template function
	// Why do we want that as a non-template function? To avoid circular include with templated shit..
	// ABSOLUTE DOG
	myResourceManager->RequestLoading<AnimationClipResource>(myIdentifier);
}

void Engine::AnimationClipResource::LoadMe() const
{
	myResourceManager->Load<AnimationClipResource>(myIdentifier);
}

void Engine::AnimationClipResource::UnloadMe()
{
	myAnimationClip = nullptr;
}
