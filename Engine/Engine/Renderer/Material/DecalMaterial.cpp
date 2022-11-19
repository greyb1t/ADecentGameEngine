#include "pch.h"
#include "DecalMaterial.h"
#include "Engine/ResourceManagement/Resources/TextureResource.h"
#include "Engine/ResourceManagement/Resources/MaterialResource.h"

void Engine::DecalMaterial::InitFromJson(
	const nlohmann::json& aJson,
	MaterialFactoryContext& aFactoryContext,
	Engine::MaterialResource& aMaterialResource,
	const MaterialType aMaterialType)
{
	Material::InitFromJson(aJson, aFactoryContext, aMaterialResource, aMaterialType);

	if (aJson.contains("AlbedoTexture"))
	{
		myAlbedoTexture = GResourceManager->CreateRef<Engine::TextureResource>(aJson["AlbedoTexture"]);
		myAlbedoTexture->RequestLoading();
		aMaterialResource.AddChild(myAlbedoTexture);
	}

	if (aJson.contains("MaterialTexture"))
	{
		myMaterialTexture = GResourceManager->CreateRef<Engine::TextureResource>(aJson["MaterialTexture"]);
		myMaterialTexture->RequestLoading();
		aMaterialResource.AddChild(myMaterialTexture);
	}

	if (aJson.contains("NormalTexture"))
	{
		myNormalTexture = GResourceManager->CreateRef<Engine::TextureResource>(aJson["NormalTexture"]);
		myNormalTexture->RequestLoading();
		aMaterialResource.AddChild(myNormalTexture);
	}

	if (aJson.contains("EmissiveTexture"))
	{
		myEmissiveTexture = GResourceManager->CreateRef<Engine::TextureResource>(aJson["EmissiveTexture"]);
		myEmissiveTexture->RequestLoading();
		aMaterialResource.AddChild(myEmissiveTexture);
	}
}

nlohmann::json Engine::DecalMaterial::ToJson() const
{
	nlohmann::json j = Material::ToJson();

	if (myAlbedoTexture && myAlbedoTexture->IsValid())
	{
		j["AlbedoTexture"] = myAlbedoTexture->GetPath();
	}

	if (myMaterialTexture && myMaterialTexture->IsValid())
	{
		j["MaterialTexture"] = myMaterialTexture->GetPath();
	}

	if (myNormalTexture && myNormalTexture->IsValid())
	{
		j["NormalTexture"] = myNormalTexture->GetPath();
	}

	if (myEmissiveTexture && myEmissiveTexture->IsValid())
	{
		j["EmissiveTexture"] = myEmissiveTexture->GetPath();
	}

	return j;
}

const TextureRef& Engine::DecalMaterial::GetAlbedoTexture() const
{
	return myAlbedoTexture;
}

const TextureRef& Engine::DecalMaterial::GetMaterialTexture() const
{
	return myMaterialTexture;
}

const TextureRef& Engine::DecalMaterial::GetNormalTexture() const
{
	return myNormalTexture;
}

const TextureRef& Engine::DecalMaterial::GetEmissiveTexture() const
{
	return myEmissiveTexture;
}
