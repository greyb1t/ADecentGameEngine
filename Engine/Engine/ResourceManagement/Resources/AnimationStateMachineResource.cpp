#include "pch.h"
#include "AnimationStateMachineResource.h"
#include "../ResourceThreadContext.h"
#include "Engine/Animation/AnimationStateMachine.h"
#include "../ResourceObserver.h"

Engine::AnimationStateMachineResource::AnimationStateMachineResource(
	ResourceManager& aResourceManager,
	const std::string& aPath,
	const ResourceId& aId)
	: ResourceBase(aResourceManager, aPath, aId)
{
}

Engine::AnimationStateMachineResource::~AnimationStateMachineResource()
{
}

Engine::AnimationStateMachineResource::AnimationStateMachineResource(const AnimationStateMachineResource& aOther) :
	ResourceBase(*aOther.myResourceManager, aOther.myPath, aOther.myIdentifier)
{
	assert(aOther.myAnimation == nullptr && "why is this not nullptr? it is "
		"supposed to be because we only ever copy this in ResourceManager "
		"before it has been fully created");
}

Engine::AnimationStateMachine& Engine::AnimationStateMachineResource::Get() const
{
	return *myAnimation;
}

bool Engine::AnimationStateMachineResource::Create(ResourceThreadData& aThreadData)
{
	auto animation = MakeOwned<AnimationStateMachine>();

	if (!animation->Init(myPath, *this))
	{
		return false;
	}

	myAnimation = std::move(animation);

	return true;
}

void Engine::AnimationStateMachineResource::Swap(ResourceBase* aResourceBase)
{
	// We know for a fact that aResourceBase input argument is of the same class
	// otherwise something went wrong in the ResourceManager
	auto other = reinterpret_cast<AnimationStateMachineResource*>(aResourceBase);

	myAnimation.swap(other->myAnimation);
}

void Engine::AnimationStateMachineResource::PostCreation(ResourceThreadData& aResourceThreadData)
{
	myResourceManager->GetFileWatcher()->WatchFile(myPath,
		[this, &aResourceThreadData](const std::string& aPath)
	{
		// this lambda will be called from render thread (main thread)
		// so its ok to edit stuff here because renderer 
		// won't touch the sprite during this time
		Create(aResourceThreadData);

		for (auto observer : myObservers)
		{
			observer->OnResourceReloaded();
		}
	});
}

void Engine::AnimationStateMachineResource::RequestMe() const
{
	// Sadly this is required to avoid having the ResourceBase::RequestLoading as a NON-template function
	// Why do we want that as a non-template function? To avoid circular include with templated shit..
	// ABSOLUTE DOG
	myResourceManager->RequestLoading<AnimationStateMachineResource>(myIdentifier);
}

void Engine::AnimationStateMachineResource::LoadMe() const
{
	myResourceManager->Load<AnimationStateMachineResource>(myIdentifier);
}

void Engine::AnimationStateMachineResource::UnloadMe()
{
	myAnimation = nullptr;
}
