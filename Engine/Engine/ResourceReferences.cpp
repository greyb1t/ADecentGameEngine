#include "pch.h"
#include "ResourceReferences.h"
#include "Engine.h"
#include "ResourceManagement\Resources\TextureResource.h"
#include "ResourceManagement\ResourceManager.h"
#include "ResourceManagement\Resources\AnimationStateMachineResource.h"
#include "ResourceManagement\Resources\AnimationClipResource.h"
#include "ResourceManagement\Resources\ModelResource.h"
#include "ResourceManagement\Resources\MaterialResource.h"
#include "ResourceManagement\Resources\PixelShaderResource.h"
#include "ResourceManagement\Resources\VertexShaderResource.h"
#include "ResourceManagement\Resources\VisualScriptResource.h"
#include "ResourceManagement\Resources\AnimationCurveResource.h"
#include "ResourceManagement\Resources\GameObjectPrefabResource.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"

bool Engine::ResourceReferences::AddReferencesFromJson(const Path& aPath)
{
	std::ifstream file(aPath.ToWString());

	if (!file.is_open())
	{
		return false;
	}

	const bool ignoreComments = true;
	const bool allowExceptions = false;
	nlohmann::json j = nlohmann::json::parse(file, nullptr, allowExceptions, ignoreComments);

	// If failed to parse json file
	if (j.is_discarded())
	{
		LOG_ERROR(LogType::Resource) << "Failed to parse: " << aPath;
		return false;
	}

	for (const std::string path : j["Textures"])
	{
		AddTexture(path);
	}

	for (const std::string path : j["Graphs"])
	{
		AddGraph(path);
	}

	for (const std::string path : j["Models"])
	{
		AddModel(path);
	}

	for (const std::string path : j["AnimationClips"])
	{
		AddAnimationClip(path);
	}

	for (const std::string path : j["Animations"])
	{
		AddAnimation(path);
	}

	for (const std::string path : j["MeshMaterials"])
	{
		AddMeshMaterial(path);
	}

	for (const std::string path : j["AnimationCurves"])
	{
		AddAnimationCurve(path);
	}

	for (const std::string path : j["GameObjectPrefabs"])
	{
		AddGameObjectPrefab(path);
	}

	for (const std::string path : j["VFXs"])
	{
		AddVFX(path);
	}

	return true;
}

bool Engine::ResourceReferences::SaveToJson(const Path& aPath) const
{
	nlohmann::json j;

	j["Textures"] = ResourceVectorToJson(myResources, ResourceType::Texture);
	j["Models"] = ResourceVectorToJson(myResources, ResourceType::Model);
	j["AnimationClips"] = ResourceVectorToJson(myResources, ResourceType::AnimationClip);
	j["Animations"] = ResourceVectorToJson(myResources, ResourceType::AnimationStateMachine);
	j["MeshMaterials"] = ResourceVectorToJson(myResources, ResourceType::MeshMaterial);
	j["Graphs"] = ResourceVectorToJson(myResources, ResourceType::VisualScript);
	j["AnimationCurves"] = ResourceVectorToJson(myResources, ResourceType::AnimationCurve);
	j["GameObjectPrefabs"] = ResourceVectorToJson(myResources, ResourceType::GameObjectPrefab);
	j["VFXs"] = ResourceVectorToJson(myResources, ResourceType::VFX);

	FileIO::RemoveReadOnly(aPath);

	std::ofstream file(aPath.ToWString());

	if (file.is_open())
	{
		file << std::setw(4) << j;
		file.close();
	}
	else
	{
		LOG_ERROR(LogType::Engine) << "Failed to save scene, is it read only?";
		return false;
	}

	return true;
}

TextureRef Engine::ResourceReferences::AddTexture(
	const Path& aPath)
{
	auto& rm = GetEngine().GetResourceManager();
	auto resource = rm.CreateRef<TextureResource>(aPath.ToString());
	myResources.insert(resource);
	return resource;
}

VisualScriptRef Engine::ResourceReferences::AddGraph(
	const Path& aPath)
{
	auto& rm = GetEngine().GetResourceManager();
	auto resource = rm.CreateRef<VisualScriptResource>(aPath.ToString());
	myResources.insert(resource);
	return resource;
}

ModelRef Engine::ResourceReferences::AddModel(const Path& aPath)
{
	auto& rm = GetEngine().GetResourceManager();
	auto resource = rm.CreateRef<ModelResource>(aPath.ToString());
	myResources.insert(resource);
	return resource;
}

AnimationClipRef Engine::ResourceReferences::AddAnimationClip(
	const Path& aPath)
{
	auto& rm = GetEngine().GetResourceManager();
	auto resource = rm.CreateRef<AnimationClipResource>(aPath.ToString());
	myResources.insert(resource);
	return resource;
}

AnimationMachineRef Engine::ResourceReferences::AddAnimation(
	const Path& aPath)
{
	auto& rm = GetEngine().GetResourceManager();
	auto resource = rm.CreateRef<AnimationStateMachineResource>(aPath.ToString());
	myResources.insert(resource);
	return resource;
}

MaterialRef Engine::ResourceReferences::AddMeshMaterial(
	const Path& aPath)
{
	auto& rm = GetEngine().GetResourceManager();
	auto resource = rm.CreateRef<MaterialResource>(aPath.ToString());
	myResources.insert(resource);
	return resource;
}

PixelShaderRef Engine::ResourceReferences::AddPixelShader(
	const Path& aPath)
{
	auto& rm = GetEngine().GetResourceManager();
	auto resource = rm.CreateRef<PixelShaderResource>(aPath.ToString());
	myResources.insert(resource);

	return resource;
}

AnimationCurveRef Engine::ResourceReferences::AddAnimationCurve(const Path& aPath)
{
	auto& rm = GetEngine().GetResourceManager();
	auto resource = rm.CreateRef<AnimationCurveResource>(aPath.ToString());
	myResources.insert(resource);

	return resource;
}

GameObjectPrefabRef Engine::ResourceReferences::AddGameObjectPrefab(const Path& aPath)
{
	auto& rm = GetEngine().GetResourceManager();
	auto resource = rm.CreateRef<GameObjectPrefabResource>(aPath.ToString());
	myResources.insert(resource);

	return resource;
}

VFXRef Engine::ResourceReferences::AddVFX(const Path& aPath)
{
	auto& rm = GetEngine().GetResourceManager();
	auto resource = rm.CreateRef<VFXResource>(aPath.ToString());
	myResources.insert(resource);

	return resource;
}

void Engine::ResourceReferences::AddVertexShader(const VertexShaderRef& aResource)
{
	myResources.insert(aResource);
}

void Engine::ResourceReferences::AddGeneralResource(const ResourceRef<ResourceBase>& aResource)
{
	switch (aResource->GetType())
	{
	case ResourceType::AnimationClip:
		myResources.insert(aResource.Cast<AnimationClipResource>());
		break;
	case ResourceType::AnimationCurve:
		myResources.insert(aResource.Cast<AnimationCurveResource>());
		break;
	case ResourceType::GameObjectPrefab:
		myResources.insert(aResource.Cast<GameObjectPrefabResource>());
		break;
	case ResourceType::VFX:
		myResources.insert(aResource.Cast<VFXResource>());
		break;
	case ResourceType::AnimationStateMachine:
		myResources.insert(aResource.Cast<AnimationStateMachineResource>());
		break;
	case ResourceType::PixelShader:
		myResources.insert(aResource.Cast<PixelShaderResource>());
		break;
	case ResourceType::VertexShader:
		myResources.insert(aResource.Cast<VertexShaderResource>());
		break;
	case ResourceType::GeometryShader:
		break;
	case ResourceType::MeshMaterial:
		myResources.insert(aResource.Cast<MaterialResource>());
		break;
	case ResourceType::Texture:
		myResources.insert(aResource.Cast<TextureResource>());
		break;
	case ResourceType::Model:
		myResources.insert(aResource.Cast<ModelResource>());
		break;
	case ResourceType::VisualScript:
		myResources.insert(aResource.Cast<VisualScriptResource>());
		break;
	default:
		assert(false);
		break;
	}
}

void Engine::ResourceReferences::UnloadAll()
{
	for (auto& r : myResources)
	{
		ResourceRef<ResourceBase>& rr = const_cast<ResourceRef<ResourceBase>&>(r);

		rr->Unload();
	}

	//UnloadResourceVector<ResourceBase>(myResources);
	//UnloadResourceVector(myModels);
	//UnloadResourceVector(myAnimationClips);
	//UnloadResourceVector(myAnimationStateMachines);
	//UnloadResourceVector(myMeshMaterials);
	//UnloadResourceVector(myGraphs);
	//UnloadResourceVector(myPixelShaders);
	//UnloadResourceVector(myVertexShaders);
	//UnloadResourceVector(myAnimationCurves);
	//UnloadResourceVector(myGameObjectPrefabs);
	//UnloadResourceVector(myVFXs);
}

void Engine::ResourceReferences::RequestAll()
{
	RequestResourceVector(myResources);
	//RequestResourceVector(myModels);
	//RequestResourceVector(myAnimationClips);
	//RequestResourceVector(myAnimationStateMachines);
	//RequestResourceVector(myMeshMaterials);
	//RequestResourceVector(myGraphs);
	//RequestResourceVector(myPixelShaders);
	//RequestResourceVector(myVertexShaders);
	//RequestResourceVector(myAnimationCurves);
	//RequestResourceVector(myGameObjectPrefabs);
	//RequestResourceVector(myVFXs);
}

void Engine::ResourceReferences::LoadAll()
{
	LoadResourceVector(myResources);
	//LoadResourceVector(myModels);
	//LoadResourceVector(myAnimationClips);
	//LoadResourceVector(myAnimationStateMachines);
	//LoadResourceVector(myMeshMaterials);
	//LoadResourceVector(myGraphs);
	//LoadResourceVector(myPixelShaders);
	//LoadResourceVector(myVertexShaders);
	//LoadResourceVector(myAnimationCurves);
	//LoadResourceVector(myGameObjectPrefabs);
	//LoadResourceVector(myVFXs);
}

bool Engine::ResourceReferences::AreAllFinishedLoading() const
{
	if (!IsResourceVectorFinishedLoading(myResources)) return false;
	//if (!IsResourceVectorFinishedLoading(myModels)) return false;
	//if (!IsResourceVectorFinishedLoading(myAnimationClips)) return false;
	//if (!IsResourceVectorFinishedLoading(myAnimationStateMachines)) return false;
	//if (!IsResourceVectorFinishedLoading(myMeshMaterials)) return false;
	//if (!IsResourceVectorFinishedLoading(myGraphs)) return false;
	//if (!IsResourceVectorFinishedLoading(myPixelShaders)) return false;
	//if (!IsResourceVectorFinishedLoading(myVertexShaders)) return false;
	//if (!IsResourceVectorFinishedLoading(myAnimationCurves)) return false;
	//if (!IsResourceVectorFinishedLoading(myGameObjectPrefabs)) return false;
	//if (!IsResourceVectorFinishedLoading(myVFXs)) return false;

	return true;
}

const std::set<Engine::ResourceRef<Engine::ResourceBase>>& Engine::ResourceReferences::GetResources() const
{
	return myResources;
}

float Engine::ResourceReferences::GetPercentageLoaded() const
{
	return GetPercentageLoaded(myResources);
}

//std::vector<Engine::ResourceRef<Engine::ResourceBase>>& Engine::ResourceReferences::GetResources(const ResourceType aType)
//{
//	return myResources[static_cast<int>(aType)];
//}
