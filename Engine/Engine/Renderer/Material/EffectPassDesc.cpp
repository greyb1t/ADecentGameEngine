#include "pch.h"
#include "EffectPassDesc.h"

void Engine::EffectPassDesc::InitFromJson(const nlohmann::json& aJson, const MaterialType aMaterialType)
{
	if (aJson.contains("Name"))
	{
		myName = aJson["Name"];
	}
	else
	{
		myName = "Unnamed";
	}

	// TODO: Only models are using renderer type
	// not fullscreen effects, not sprites and other stuff
	if (aJson.contains("Renderer"))
	{
		const std::string renderer = aJson["Renderer"];

		if (renderer == "Deferred")
		{
			myRenderingType = RendererType::Deferred;
		}
		else
		{
			myRenderingType = RendererType::Forward;
		}
	}

	if (aJson.contains("Renderer2"))
	{
		myRenderingType = aJson["Renderer2"];
	}

	myVertexShaderNamePath = aJson["VertexShaderNamePath"];
	myPixelShaderNamePath = aJson["PixelShaderNamePath"];
	
	if (aJson.contains("GeometryShaderNamePath"))
	{
		myGeometryShaderNamePath = aJson["GeometryShaderNamePath"];
	}

	if (aJson.contains("VertexType"))
	{
		myVertexType = aJson["VertexType"];

		if (myVertexType == "DefaultVertex")
		{
			myInputLayoutDesc = DefaultVertex::ourInputElements;
			myInputLayoutElementCount = DefaultVertex::ourElementCount;
		}
		else if (myVertexType == "SpriteVertex")
		{
			myInputLayoutDesc = SpriteVertex::ourInputElements;
			myInputLayoutElementCount = SpriteVertex::ourElementCount;
		}
		else if (myVertexType == "ParticleVertex")
		{
			myInputLayoutDesc = ParticleVertex::ourInputElements;
			myInputLayoutElementCount = ParticleVertex::ourElementCount;
		}
	}
	else
	{
		switch (aMaterialType)
		{
		case MaterialType::Mesh:
			myInputLayoutDesc = DefaultVertex::ourInputElements;
			myInputLayoutElementCount = DefaultVertex::ourElementCount;
			break;
		case MaterialType::Sprite:
			myInputLayoutDesc = SpriteVertex::ourInputElements;
			myInputLayoutElementCount = SpriteVertex::ourElementCount;
			break;
		case MaterialType::Decal:
			// use the default mesh vertex for decauls
			myInputLayoutDesc = DefaultVertex::ourInputElements;
			myInputLayoutElementCount = DefaultVertex::ourElementCount;
			break;
		case MaterialType::Particle:
			myInputLayoutDesc = ParticleVertex::ourInputElements;
			myInputLayoutElementCount = ParticleVertex::ourElementCount;
			break;
		default:
			break;
		}

		/*
		myVertexType = "DefaultVertex";

		myInputLayoutDesc = DefaultVertex::ourInputElements;
		myInputLayoutElementCount = DefaultVertex::ourElementCount;
		*/
	}

	if (aJson.contains("Textures"))
	{
		for (const auto& textureJson : aJson["Textures"])
		{
			EffectTextureDesc texture;
			texture.InitFromJson(textureJson);

			myTextures.push_back(texture);
		}
	}

	if (aJson.contains("Stencil"))
	{
		myStencil.InitFromJson(aJson["Stencil"]);
	}

	if (aJson.contains("RasterizerState"))
	{
		myRasterizerState = aJson["RasterizerState"];
	}

	if (aJson.contains("BlendState"))
	{
		assert(aJson["BlendState"].is_number_integer() &&
			"An old blend state, change it to an int in the .material or ask filip");

		myBlendState = aJson["BlendState"];
	}

	for (const auto& constantBufferJson : aJson["ConstantBuffers"])
	{
		EffectConstantBufferDesc constantBuffer;
		constantBuffer.InitFromJson(constantBufferJson);

		const auto findResult = std::find_if(
			myConstantBuffers.begin(),
			myConstantBuffers.end(),
			[&](const EffectConstantBufferDesc& aLoaderConstantBuffer)
			{
				return aLoaderConstantBuffer.myName == constantBuffer.myName;
			});

		const bool exists = findResult != myConstantBuffers.end();

		if (exists)
		{
			LOG_ERROR(LogType::Renderer) << "Duplicate ConstantBuffer " << constantBuffer.myName << "in Materials.json";
		}
		else
		{
			myConstantBuffers.push_back(constantBuffer);
		}
	}

	if (aJson.contains("ShaderDefines"))
	{
		for (const std::string shaderDefine : aJson["ShaderDefines"])
		{
			myShaderDefines.push_back(shaderDefine);
		}
	}

	if (aJson.contains("EmissionIntensity"))
	{
		myEmissionIntensity = aJson["EmissionIntensity"];
	}
}

nlohmann::json Engine::EffectPassDesc::ToJson() const
{
	nlohmann::json j;

	nlohmann::json texturesJson = nlohmann::json::array();

	for (const auto& loaderTexture : myTextures)
	{
		texturesJson.push_back(loaderTexture.ToJson());
	}

	j["Textures"] = texturesJson;

	j["VertexShaderNamePath"] = myVertexShaderNamePath;

	j["PixelShaderNamePath"] = myPixelShaderNamePath;

	j["GeometryShaderNamePath"] = myGeometryShaderNamePath;

	nlohmann::json constantBuffersJson = nlohmann::json::array();

	for (const auto& constantBuffer : myConstantBuffers)
	{
		constantBuffersJson.push_back(constantBuffer.ToJson());
	}

	j["ConstantBuffers"] = constantBuffersJson;

	j["Renderer2"] = myRenderingType;

	j["EmissionIntensity"] = myEmissionIntensity;

	j["Stencil"] = myStencil.ToJson();

	j["RasterizerState"] = myRasterizerState;

	j["BlendState"] = myBlendState;

	j["ShaderDefines"] = myShaderDefines;
	
	j["VertexType"] = myVertexType;

	return j;
}

void Engine::EffectConstantBufferDesc::Layout::InitFromJson(const nlohmann::json& j)
{
	for (const auto& variableJson : j)
	{
		Variable var;

		var.InitFromJson(variableJson);

		const auto findResult = std::find_if(
			myVariables.begin(),
			myVariables.end(),
			[&](const Variable& aLoaderVariable)
			{
				return aLoaderVariable.myName == var.myName;
			});

		const bool exists = findResult != myVariables.end();

		if (exists)
		{
			LOG_ERROR(LogType::Renderer) << "Duplicate variable \"" << var.myName << "\" in ConstantBuffer in Materials.json";
		}
		else
		{
			myVariables.push_back(var);
		}
	}
}

nlohmann::json Engine::EffectConstantBufferDesc::Layout::ToJson() const
{
	nlohmann::json j = nlohmann::json::array();

	for (const auto& variable : myVariables)
	{
		j.push_back(variable.ToJson());
	}

	return j;
}
