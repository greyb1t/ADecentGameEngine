#include "pch.h"
#include "MeshMaterial.h"
#include "Engine\ResourceManagement\ResourceManager.h"
#include "Engine/ResourceManagement/Resources/TextureResource.h"
#include "Engine/ResourceManagement/Resources/MaterialResource.h"
#include "Engine\Renderer\Texture\Texture2D.h"

void Engine::MeshMaterial::InitFromJson(
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

nlohmann::json Engine::MeshMaterial::ToJson() const
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

void Engine::MeshMaterial::Bind(ID3D11DeviceContext& aContext)
{
	ID3D11ShaderResourceView* textures[] =
	{
		myAlbedoTexture ? myAlbedoTexture->Get().GetSRV() : nullptr,
		myNormalTexture ? myNormalTexture->Get().GetSRV() : nullptr,
		myMaterialTexture ? myMaterialTexture->Get().GetSRV() : nullptr,
		myEmissiveTexture ? myEmissiveTexture->Get().GetSRV() : nullptr,
	};

	aContext.PSSetShaderResources(
		1,
		std::size(textures),
		&textures[0]);
}
