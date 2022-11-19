#include "pch.h"
#include "MaterialResource.h"
#include "../ResourceThreadContext.h"
#include "Engine/Engine.h"
#include "Engine/Renderer/GraphicsEngine.h"
#include "Engine/Renderer/Animation/AnimationClipLoader.h"
#include "Engine/Renderer/Material/MaterialFactory.h"
#include "Engine/Renderer/Material/MeshMaterial.h"
#include "Engine/Renderer/Material/DecalMaterial.h"
#include "Engine/Renderer/Material/ParticleMaterial.h"

Engine::MaterialResource::MaterialResource(
	ResourceManager& aResourceManager,
	const std::string& aPath,
	const ResourceId& aId)
	: ResourceBase(aResourceManager, aPath, aId)
{
	if (aPath == "DefaultModelMaterial")
	{
		int test = 0;
	}
}

Engine::MaterialResource::~MaterialResource()
{
}

Engine::MaterialResource::MaterialResource(const MaterialResource& aOther) :
	ResourceBase(*aOther.myResourceManager, aOther.myPath, aOther.myIdentifier)
{
	assert(aOther.myMeshMaterial == nullptr && "why is this not nullptr? it is "
		"supposed to be because we only ever copy this in ResourceManager "
		"before it has been fully created");
}

Engine::Material* Engine::MaterialResource::Get() const
{
	return myMeshMaterial.get();
}

Engine::MeshMaterial* Engine::MaterialResource::GetAsMeshMaterial()
{
	if (myMeshMaterial->GetType() == MaterialType::Mesh)
	{
		return static_cast<MeshMaterial*>(myMeshMaterial.get());
	}

	return nullptr;
}

Engine::DecalMaterial* Engine::MaterialResource::GetAsDecalMaterial() const
{
	if (myMeshMaterial->GetType() == MaterialType::Decal)
	{
		return static_cast<DecalMaterial*>(myMeshMaterial.get());
	}

	return nullptr;
}

Engine::ParticleMaterial* Engine::MaterialResource::GetAsParticleMaterial() const
{
	if (myMeshMaterial->GetType() == MaterialType::Particle)
	{
		return static_cast<ParticleMaterial*>(myMeshMaterial.get());
	}

	return nullptr;
}

Engine::ParticleMaterial* Engine::MaterialResource::GetAsParticleMaterial()
{
	if (myMeshMaterial->GetType() == MaterialType::Particle)
	{
		return static_cast<ParticleMaterial*>(myMeshMaterial.get());
	}

	return nullptr;
}

Engine::DecalMaterial* Engine::MaterialResource::GetAsDecalMaterial()
{
	if (myMeshMaterial->GetType() == MaterialType::Decal)
	{
		return static_cast<DecalMaterial*>(myMeshMaterial.get());
	}

	return nullptr;
}

Engine::MeshMaterial* Engine::MaterialResource::GetAsMeshMaterial() const
{
	if (myMeshMaterial->GetType() == MaterialType::Mesh)
	{
		return static_cast<MeshMaterial*>(myMeshMaterial.get());
	}

	return nullptr;
}

Engine::Material* Engine::MaterialResource::Get()
{
	return myMeshMaterial.get();
}

bool Engine::MaterialResource::Create(ResourceThreadData& aThreadData)
{
	std::ifstream file(myPath);

	if (!file.is_open())
	{
		LOG_ERROR(LogType::Resource) << "Failed to open " << myPath;
		return false;
	}

	const bool ignoreComments = true;
	const bool allowExceptions = false;
	auto myJson = nlohmann::json::parse(file, nullptr, allowExceptions, ignoreComments);

	// If failed to parse json file
	if (myJson.is_discarded())
	{
		LOG_ERROR(LogType::Resource) << "Failed to parse " << myPath;
		return false;
	}

	if (myJson.contains("Type"))
	{
		const MaterialType materialType = myJson["Type"];

		switch (materialType)
		{
		case MaterialType::Mesh:
			myMeshMaterial = MakeOwned<MeshMaterial>();
			break;
		case MaterialType::Sprite:
			// NOTE: atm sprite material is just a mesh material interally
			myMeshMaterial = MakeOwned<MeshMaterial>();
			break;
		case MaterialType::Decal:
			myMeshMaterial = MakeOwned<DecalMaterial>();
			break;
		case MaterialType::Particle:
			myMeshMaterial = MakeOwned<ParticleMaterial>();
			break;
		default:
			LOG_ERROR(LogType::Resource) << "Unknown material type: " << myPath;
			break;
		}

		// TODO: not thread safe
		auto& ctx = GetEngine().GetGraphicsEngine().GetMaterialFactory().GetFactoryContext();
		myMeshMaterial->InitFromJson(myJson, ctx, *this, materialType);
	}
	else
	{
		LOG_ERROR(LogType::Resource) << "Material missing \"Type\" in json, skipping it" << myPath;
		return false;
	}

	return true;
}

void Engine::MaterialResource::Swap(ResourceBase* aResourceBase)
{
	// We know for a fact that aResourceBase input argument is of the same class
	// otherwise something went wrong in the ResourceManager
	auto other = reinterpret_cast<MaterialResource*>(aResourceBase);

	myMeshMaterial.swap(other->myMeshMaterial);
}

void Engine::MaterialResource::PostCreation(ResourceThreadData& aResourceThreadData)
{
	// NOTE(filip): disabled file watcher re-creation because I added
	// material editing within the editor

	//myResourceManager->GetFileWatcher()->WatchFile(myPath,
	//	[this, &aResourceThreadData](const std::string& aPath)
	//	{
	//		// this lambda will be called from render thread (main thread)
	//		// so its ok to edit stuff here because renderer 
	//		// won't touch the sprite during this time
	//		Create(aResourceThreadData);
	//	});
}

void Engine::MaterialResource::RequestMe() const
{
	// Sadly this is required to avoid having the ResourceBase::RequestLoading as a NON-template function
	// Why do we want that as a non-template function? To avoid circular include with templated shit..
	// ABSOLUTE DOG
	myResourceManager->RequestLoading<MaterialResource>(myIdentifier);
}

void Engine::MaterialResource::LoadMe() const
{
	myResourceManager->Load<MaterialResource>(myIdentifier);
}

void Engine::MaterialResource::UnloadMe()
{
	myMeshMaterial = nullptr;
}
