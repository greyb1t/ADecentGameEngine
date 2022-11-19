#include "pch.h"
#include "ParticleMaterial.h"
#include "Engine/ResourceManagement/Resources/TextureResource.h"
#include "Engine/ResourceManagement/Resources/MaterialResource.h"

void Engine::ParticleMaterial::InitFromJson(
	const nlohmann::json& aJson,
	MaterialFactoryContext& aFactoryContext,
	Engine::MaterialResource& aMaterialResource,
	const MaterialType aMaterialType)
{
	Material::InitFromJson(aJson, aFactoryContext, aMaterialResource, aMaterialType);
}