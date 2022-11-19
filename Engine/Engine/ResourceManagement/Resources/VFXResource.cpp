#include "pch.h"
#include "VFXResource.h"
#include "../ResourceThreadContext.h"
#include "Engine/GraphManager/VisualScript.h"
#include "../ResourceObserver.h"
#include "Engine/VFX/VFXUtilities.h"
#include "Engine/VFX/Description/ParticleEmitterDescription.h"

Engine::VFXResource::VFXResource(
	ResourceManager& aResourceManager,
	const std::string& aPath,
	const ResourceId& aId)
	: ResourceBase(aResourceManager, aPath, aId)
{
}

Engine::VFXResource::~VFXResource()
{
}

Engine::VFXResource::VFXResource(const VFXResource& aOther) :
	ResourceBase(*aOther.myResourceManager, aOther.myPath, aOther.myIdentifier)
{
	assert(aOther.myVFXDesc == nullptr && "why is this not nullptr? it is "
		"supposed to be because we only ever copy this in ResourceManager "
		"before it has been fully created");
}

VFX::VFXDescription& Engine::VFXResource::Get() const
{
	return *myVFXDesc;
}

bool Engine::VFXResource::Create(ResourceThreadData& aThreadData)
{
	bool succeeded = false;
	auto vfxDesc = MakeOwned<VFX::VFXDescription>(VFX::Load(myPath, &succeeded));

	// Must add as child to this resource to ensure they are checked
	// if they have finished loading
	for (auto& emitter : vfxDesc->particleEmitters.GetKeys())
	{
		AddChild(emitter.data.myRenderData.myGSShader);
		AddChild(emitter.data.myRenderData.myTexture);
	}

	if (!succeeded)
	{
		return false;
	}

	myVFXDesc = std::move(vfxDesc);

	return true;
}

void Engine::VFXResource::Swap(ResourceBase* aResourceBase)
{
	// We know for a fact that aResourceBase input argument is of the same class
	// otherwise something went wrong in the ResourceManager
	auto other = reinterpret_cast<VFXResource*>(aResourceBase);

	myVFXDesc.swap(other->myVFXDesc);
}

void Engine::VFXResource::PostCreation(ResourceThreadData& aResourceThreadData)
{
	myResourceManager->GetFileWatcher()->WatchFile(myPath,
		[this, &aResourceThreadData](const std::string& aPath)
		{
			Create(aResourceThreadData);
		});
}

void Engine::VFXResource::RequestMe() const
{
	// Sadly this is required to avoid having the ResourceBase::RequestLoading as a NON-template function
	// Why do we want that as a non-template function? To avoid circular include with templated shit..
	// ABSOLUTE DOG
	myResourceManager->RequestLoading<VFXResource>(myIdentifier);
}

void Engine::VFXResource::LoadMe() const
{
	myResourceManager->Load<VFXResource>(myIdentifier);
}

void Engine::VFXResource::UnloadMe()
{
	myVFXDesc = nullptr;
}
