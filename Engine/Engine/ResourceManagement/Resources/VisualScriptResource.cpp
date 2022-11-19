#include "pch.h"
#include "VisualScriptResource.h"
#include "../ResourceThreadContext.h"
#include "Engine/GraphManager/VisualScript.h"
#include "../ResourceObserver.h"

Engine::VisualScriptResource::VisualScriptResource(
	ResourceManager& aResourceManager,
	const std::string& aPath,
	const ResourceId& aId)
	: ResourceBase(aResourceManager, aPath, aId)
{
}

Engine::VisualScriptResource::~VisualScriptResource()
{
}

Engine::VisualScriptResource::VisualScriptResource(const VisualScriptResource& aOther) :
	ResourceBase(*aOther.myResourceManager, aOther.myPath, aOther.myIdentifier)
{
	assert(aOther.myVisualGraph == nullptr && "why is this not nullptr? it is "
		"supposed to be because we only ever copy this in ResourceManager "
		"before it has been fully created");
}

Engine::VisualScript& Engine::VisualScriptResource::Get() const
{
	return *myVisualGraph;
}

bool Engine::VisualScriptResource::Create(ResourceThreadData& aThreadData)
{
	auto visualGraph = MakeOwned<VisualScript>();

	if (!visualGraph->Init(myPath))
	{
		return false;
	}

	myVisualGraph = std::move(visualGraph);

	return true;
}

void Engine::VisualScriptResource::Swap(ResourceBase* aResourceBase)
{
	// We know for a fact that aResourceBase input argument is of the same class
	// otherwise something went wrong in the ResourceManager
	auto other = reinterpret_cast<VisualScriptResource*>(aResourceBase);

	myVisualGraph.swap(other->myVisualGraph);
}

void Engine::VisualScriptResource::PostCreation(ResourceThreadData& aResourceThreadData)
{
	myResourceManager->GetFileWatcher()->WatchFile(myPath,
		[this, &aResourceThreadData](const std::string& aPath)
		{
			Create(aResourceThreadData);
		});
}

void Engine::VisualScriptResource::RequestMe() const
{
	// Sadly this is required to avoid having the ResourceBase::RequestLoading as a NON-template function
	// Why do we want that as a non-template function? To avoid circular include with templated shit..
	// ABSOLUTE DOG
	myResourceManager->RequestLoading<VisualScriptResource>(myIdentifier);
}

void Engine::VisualScriptResource::LoadMe() const
{
	myResourceManager->Load<VisualScriptResource>(myIdentifier);
}

void Engine::VisualScriptResource::UnloadMe()
{
	myVisualGraph = nullptr;
}
