#include "pch.h"
#include "ModelResource.h"
#include "Engine/Renderer/Shaders/ShaderLoader.h"
#include "Engine/Renderer/Shaders/PixelShader.h"
#include "Engine/Renderer/Model/Model.h"
#include "../ResourceThreadContext.h"
#include "Engine/Renderer/Model/ModelFactory.h"
#include "Engine/Renderer/ModelLoader/FbxLoader.h"

Engine::ModelResource::ModelResource(
	ResourceManager& aResourceManager,
	const std::string& aPath,
	const ResourceId& aId)
	: ResourceBase(aResourceManager, aPath, aId)
{
}

Engine::ModelResource::~ModelResource()
{
}

Engine::ModelResource::ModelResource(const ModelResource& aOther) :
	ResourceBase(*aOther.myResourceManager, aOther.myPath, aOther.myIdentifier)
{
	assert(aOther.myModel == nullptr && "why is this not nullptr? it is "
		"supposed to be because we only ever copy this in ResourceManager "
		"before it has been fully created");
}

Engine::Model& Engine::ModelResource::Get() const
{
	return *myModel;
}

bool Engine::ModelResource::Create(ResourceThreadData& aThreadData)
{
	auto fbx = aThreadData.GetFbxLoader().LoadFbx(myPath, *this);

	if (fbx == nullptr)
	{
		return false;
	}

	myModel = std::move(fbx);

	return true;
}

void Engine::ModelResource::Swap(ResourceBase* aResourceBase)
{
	// We know for a fact that aResourceBase input argument is of the same class
	// otherwise something went wrong in the ResourceManager
	auto other = reinterpret_cast<ModelResource*>(aResourceBase);

	myModel.swap(other->myModel);
}

void Engine::ModelResource::PostCreation(ResourceThreadData& aResourceThreadData)
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

void Engine::ModelResource::RequestMe() const
{
	// Sadly this is required to avoid having the ResourceBase::RequestLoading as a NON-template function
	// Why do we want that as a non-template function? To avoid circular include with templated shit..
	// ABSOLUTE DOG
	myResourceManager->RequestLoading<ModelResource>(myIdentifier);
}

void Engine::ModelResource::LoadMe() const
{
	myResourceManager->Load<ModelResource>(myIdentifier);
}

void Engine::ModelResource::UnloadMe()
{
	myModel = nullptr;
}
