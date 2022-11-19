#include "pch.h"
#include "Material.h"
#include "Engine\ResourceManagement\Resources\TextureResource.h"
#include "Engine\Renderer\Texture\Texture2D.h"
#include "Engine\ResourceManagement\Resources\PixelShaderResource.h"
#include "Engine\ResourceManagement\Resources\VertexShaderResource.h"
#include "Engine\Renderer\Shaders\PixelShader.h"
#include "Engine/Renderer\VertexTypes.h"
#include "Engine\ResourceManagement\ResourceManager.h"
#include "EffectPass.h"

Engine::Material::Material()
{
}

Engine::Material::~Material()
{
}

void Engine::Material::InitFromJson(
	const nlohmann::json& aJson,
	MaterialFactoryContext& aFactoryContext,
	Engine::MaterialResource& aMaterialResource,
	const MaterialType aMaterialType)
{
	// myName = aJson["UnrealName"];

	if (aJson.contains("Type"))
	{
		myType = aJson["Type"];
	}

	if (aJson.contains("IsTransparent"))
	{
		myIsTransparent = aJson["IsTransparent"];
	}

	if (aJson.contains("CastShadows"))
	{
		myIsCastingShadows = aJson["CastShadows"];
	}

	int effectPassIndex = 0;

	for (const auto& effectPassJson : aJson["Effect"]["Passes"])
	{
		auto effectPass = MakeOwned<EffectPass>();

		effectPass->InitFromJson(
			effectPassJson,
			aFactoryContext,
			aMaterialType);

		myPasses.push_back(std::move(effectPass));

		++effectPassIndex;
	}
}

nlohmann::json Engine::Material::ToJson() const
{
	nlohmann::json j;

	// j["UnrealName"] = myName;

	if (!myPasses.empty())
	{
		nlohmann::json effectPassesArrayJson = nlohmann::json::array();

		for (const auto& pass : myPasses)
		{
			effectPassesArrayJson.push_back(pass->ToJson());
		}

		j["Effect"]["Passes"] = effectPassesArrayJson;
	}

	j["IsTransparent"] = myIsTransparent;
	j["CastShadows"] = myIsCastingShadows;
	j["Type"] = myType;

	return j;
}

void Engine::Material::RequestMaterial() const
{
	for (const auto& pass : myPasses)
	{
		pass->GetPixelShader()->RequestLoading();
		pass->GetVertexShader()->RequestLoading();
	}
}

// const std::string& Engine::Material::GetName() const
// {
// 	return myName;
// }

const std::vector<Owned<Engine::EffectPass>>& Engine::Material::GetPasses() const
{
	return myPasses;
}

void Engine::Material::SetFloat4(const std::string& aVariableName, const C::Vector4f& aValue)
{
	for (auto& pass : myPasses)
	{
		pass->SetFloat4(aVariableName, aValue);
	}
}

C::Vector4f Engine::Material::GetFloat4(const std::string& aVariableName)
{
	for (auto& pass : myPasses)
	{
		auto float4 = pass->GetFloat4(aVariableName);

		if (float4)
		{
			return *float4;
		}
	}

	LOG_WARNING(LogType::Renderer) << "Material::GetFloat4() could not find variable " << aVariableName;

	return {};
}

// const Engine::LoaderMaterial& Engine::Material::GetLoaderMaterial() const
// {
// 	return myLoaderMaterial;
// }

bool Engine::Material::IsTransparent() const
{
	return myIsTransparent;
}

Engine::MaterialType Engine::Material::GetType() const
{
	return myType;
}

bool Engine::Material::IsCastingShadows() const
{
	return myIsCastingShadows;
}
