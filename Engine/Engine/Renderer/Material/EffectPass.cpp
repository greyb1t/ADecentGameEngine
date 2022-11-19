#include "pch.h"
#include "EffectPass.h"
#include "EffectPassDesc.h"
#include "Engine\ResourceManagement\Resources\VertexShaderResource.h"
#include "..\VertexTypes.h"
#include "Engine\ResourceManagement\Resources\PixelShaderResource.h"
#include "Engine\ResourceManagement\Resources\GeometryShaderResource.h"
#include "Engine/Renderer/Shaders/PixelShader.h"
#include "Engine/Renderer/Shaders/VertexShader.h"
#include "Engine/Renderer/Shaders/GeometryShader.h"
#include "Engine/Renderer/Texture/Texture2D.h"
#include "EffectConstantBuffer.h"
#include "EffectTexture.h"
#include "Engine/ResourceManagement/Resources/TextureResource.h"
#include "MaterialFactoryContext.h"
#include "Engine/Engine.h"
#include "Engine/Renderer/GraphicsEngine.h"
#include "Engine/Renderer/RenderManager.h"
#include <filesystem>
#include "Engine\Renderer\Shaders\ShaderConstants.h"

namespace Engine
{
	std::atomic_int myEffectIDCounter = 0;
}

Engine::EffectPass::EffectPass()
{
	myID = myEffectIDCounter++;
}

Engine::EffectPass::~EffectPass()
{
	if (myVertexShader)
	{
		myVertexShader->RemoveObserver(this);
	}

	if (myGeometryShader)
	{
		myGeometryShader->RemoveObserver(this);
	}

	if (myPixelShader)
	{
		myPixelShader->RemoveObserver(this);
	}
}

bool Engine::EffectPass::InitFromLoaderEffectPass(
	MaterialFactoryContext& aFactoryContext,
	const EffectPassDesc& aDesc)
{
	myContext = aFactoryContext;

	myDesc = aDesc;

	if (!aDesc.myStencil.myType.empty())
	{
		if (aDesc.myStencil.myType == "StencilAlwaysWithDepthTest")
		{
			myDepthStencilState = DepthStencilState_StencilAlwaysWithDepthTest;
		}
		else if (aDesc.myStencil.myType == "StencilNotEqualWithDepthTest")
		{
			myDepthStencilState = DepthStencilState_StencilNotEqualWithDepthTest;
		}
		else if (aDesc.myStencil.myType == "SkyboxDepthStencil")
		{
			myDepthStencilState = DepthStencilState_Skybox;
		}
		else
		{
			LOG_WARNING(LogType::Renderer) << "Unable to find the stencil: " << aDesc.myStencil.myType;
		}

		myStencilRef = aDesc.myStencil.myRef;
	}

	/*
	if (aLoaderEffectPass.myRasterizerState != RasterizerState_NotSet)
	{
		const std::string cullMode = StringUtilities::ToLower(aLoaderEffectPass.myCullMode);

		if (cullMode == "none")
		{
			myRasterizerState = RasterizerState_NoCulling;
		}
		else if (cullMode == "cw")
		{
			myRasterizerState = RasterizerState_CW;
		}
		else if (cullMode == "ccw")
		{
			myRasterizerState = RasterizerState_CCW;
		}
	}
	*/

	/*
	if (!aLoaderEffectPass.myBlendState.empty())
	{
		const std::string blendState = StringUtilities::ToLower(aLoaderEffectPass.myBlendState);

		if (blendState == "disable")
		{
			myBlendState = BlendState_Disable;
		}
		else if (blendState == "alphablend")
		{
			myBlendState = BlendState_AlphaBlend;
		}
		else if (blendState == "alphablend_alphatocoverage")
		{
			myBlendState = BlendState_AlphaBlend_AlphaToCoverage;
		}
		else if (blendState == "additive")
		{
			myBlendState = BlendState_Additive;
		}
	}
	*/

	std::vector<ShaderDefines> defines;

	for (const auto& define : aDesc.myShaderDefines)
	{
		if (define == "INSTANCED")
		{
			defines.push_back(ShaderDefines::Instanced);

			myIsInstanced = true;
		}
	}

	myVertexShader =
		aFactoryContext.myResourceManager->CreateRefWithID<VertexShaderResource>(
			aDesc.myVertexShaderNamePath,
			ResourceId(aDesc.myVertexShaderNamePath + std::to_string(OrDefines(defines))),
			aDesc.myInputLayoutDesc,
			aDesc.myInputLayoutElementCount,
			defines);
	myVertexShader->Load();

	myVertexShader->AddObserver(this);

	if (!myVertexShader->IsValid())
	{
		LOG_ERROR(LogType::Resource) << "Failed to load vertex shader " << aDesc.myVertexShaderNamePath;
		return false;
	}

	myPixelShader =
		aFactoryContext.myResourceManager->CreateRef<PixelShaderResource>(
			aDesc.myPixelShaderNamePath);
	myPixelShader->Load();

	myPixelShader->AddObserver(this);

	if (!myPixelShader->IsValid())
	{
		LOG_ERROR(LogType::Resource) << "Failed to load pixel shader " << aDesc.myPixelShaderNamePath;
		return false;
	}

	if (!aDesc.myGeometryShaderNamePath.empty())
	{
		myGeometryShader =
			aFactoryContext.myResourceManager->CreateRef<GeometryShaderResource>(
				aDesc.myGeometryShaderNamePath);
		myGeometryShader->Load();

		myGeometryShader->AddObserver(this);

		if (!myGeometryShader->IsValid())
		{
			LOG_ERROR(LogType::Resource) << "Failed to load geometry shader " << aDesc.myPixelShaderNamePath;
			return false;
		}

		assert(myGeometryShader->IsValid() && "must be loaded bcuz of shader reflection");
	}

	assert(myVertexShader->IsValid() && "must be loaded bcuz of shader reflection");
	assert(myPixelShader->IsValid() && "must be loaded bcuz of shader reflection");

	ParseReflection(
		*aFactoryContext.myResourceManager,
		aFactoryContext.myDevice,
		*myPixelShader->Get().GetReflection(),
		ShaderFlags_PixelShader);

	ParseReflection(
		*aFactoryContext.myResourceManager,
		aFactoryContext.myDevice,
		*myVertexShader->Get().GetReflection(),
		ShaderFlags_VertexShader);

	if (myGeometryShader && myGeometryShader->IsValid())
	{
		ParseReflection(
			*aFactoryContext.myResourceManager,
			aFactoryContext.myDevice,
			*myGeometryShader->Get().GetReflection(),
			ShaderFlags_GeometryShader);
	}

	Validate();

	CreateMaterialConstantBuffer(aFactoryContext);

	// UpdateHash();

	return true;
}

void Engine::EffectPass::InitFromJson(
	const nlohmann::json& aJson,
	MaterialFactoryContext& aFactoryContext,
	const MaterialType aMaterialType)
{
	myDesc.InitFromJson(aJson, aMaterialType);

	InitFromLoaderEffectPass(aFactoryContext, myDesc);
}

nlohmann::json Engine::EffectPass::ToJson() const
{
	return myDesc.ToJson();
}

const std::string& Engine::EffectPass::GetName() const
{
	return myDesc.myName;
}

const VertexShaderRef& Engine::EffectPass::GetVertexShader() const
{
	return myVertexShader;
}

const PixelShaderRef& Engine::EffectPass::GetPixelShader() const
{
	return myPixelShader;
}

const GeometryShaderRef& Engine::EffectPass::GetGeometryShader() const
{
	return myGeometryShader;
}

Engine::EffectPassState Engine::EffectPass::BindToPipeline(
	ID3D11DeviceContext* aContext,
	const BindEffectFlags aFlags) const
{
	if ((aFlags & BindEffectFlags_PixelShader) != 0)
	{
		aContext->PSSetShader(
			myPixelShader->Get().GetPixelShader(), nullptr, 0);
	}

	if ((aFlags & BindEffectFlags_VertexShader) != 0)
	{
		aContext->IASetInputLayout(
			myVertexShader->Get().GetInputLayout());

		aContext->VSSetShader(
			myVertexShader->Get().GetVertexShader(), nullptr, 0);
	}

	if ((aFlags & BindEffectFlags_GeometryShader) != 0)
	{
		aContext->GSSetShader(
			myGeometryShader->Get().GetGeometryShader(), nullptr, 0);
	}

	// The reason for this is to be able to reset specific 
	// states that is bound by this effect
	// Example: We need to reset the depth stencil
	EffectPassState effectState;

	if ((aFlags & BindEffectFlags_EffectSpecificStuff) != 0)
	{
		if (myDepthStencilState != DepthStencilState_NotSet)
		{
			auto& renderManager = GetEngine().GetGraphicsEngine().GetRenderManager();

			ComPtr<ID3D11DepthStencilState> depthStencilState;
			UINT stencilRef = 0;

			// increases ref count, release it immediately using ComPtr
			aContext->OMGetDepthStencilState(&depthStencilState, &stencilRef);

			effectState.SetDepthStencil(depthStencilState.Get(), stencilRef);

			aContext->OMSetDepthStencilState(
				renderManager.GetDepthStencilState(myDepthStencilState),
				myStencilRef);
		}

		if (myDesc.myRasterizerState != RasterizerState_NotSet)
		{
			auto& renderManager = GetEngine().GetGraphicsEngine().GetRenderManager();

			ComPtr<ID3D11RasterizerState> rasterizerState;

			// increases ref count, release it immediately using ComPtr
			aContext->RSGetState(&rasterizerState);

			effectState.SetRasterizerState(rasterizerState.Get());

			aContext->RSSetState(renderManager.GetRasterizerState(myDesc.myRasterizerState));
		}

		if (myDesc.myBlendState != BlendState_NotSet)
		{
			auto& renderManager = GetEngine().GetGraphicsEngine().GetRenderManager();

			ComPtr<ID3D11BlendState> blendState;

			// increases ref count, release it immediately using ComPtr
			aContext->OMGetBlendState(&blendState, nullptr, nullptr);

			effectState.SetBlendState(blendState.Get());

			UINT mask = 0xffffffff;
			const FLOAT blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };

			aContext->OMSetBlendState(renderManager.GetBlendState(myDesc.myBlendState), blendFactor, mask);
		}
	}

	if ((aFlags & (BindEffectFlags_VertexShader | BindEffectFlags_PixelShader)) != 0)
	{
		// If binding the effect for both vertex and pixel shaders
		// bind everything because we'll bind them both anyway

		for (const auto& [_, constantBuffer] : myConstantBuffers)
		{
			constantBuffer->BindToPipeline(aContext);
		}

		for (const auto& texture : myTextures)
		{
			aContext->VSSetShaderResources(texture.GetShaderSlot(), 1, texture.GetTextureResource()->Get().GetSRVPtr());
			aContext->PSSetShaderResources(texture.GetShaderSlot(), 1, texture.GetTextureResource()->Get().GetSRVPtr());
		}
	}
	else if ((aFlags & BindEffectFlags_VertexShader) != 0)
	{
		// Only bind the constant buffers that is used in the vertex shaders
		for (const auto& [_, constantBuffer] : myConstantBuffers)
		{
			if (constantBuffer->IsUsedByShader(ShaderFlags_VertexShader))
			{
				constantBuffer->BindToPipeline(aContext);
			}
		}

		// only bind textures if used in vertex shader
		for (const auto& texture : myTextures)
		{
			if (!texture.IsUsedByShader(ShaderFlags_VertexShader))
			{
				continue;
			}

			aContext->VSSetShaderResources(texture.GetShaderSlot(), 1, texture.GetTextureResource()->Get().GetSRVPtr());
			aContext->PSSetShaderResources(texture.GetShaderSlot(), 1, texture.GetTextureResource()->Get().GetSRVPtr());
		}
	}
	else if ((aFlags & BindEffectFlags_PixelShader) != 0)
	{
		// Only bind the constant buffers that is used in the pixel shaders
		for (const auto& [_, constantBuffer] : myConstantBuffers)
		{
			if (constantBuffer->IsUsedByShader(ShaderFlags_PixelShader))
			{
				constantBuffer->BindToPipeline(aContext);
			}
		}

		// only bind textures if used in pixel shader
		for (const auto& texture : myTextures)
		{
			if (!texture.IsUsedByShader(ShaderFlags_PixelShader))
			{
				continue;
			}

			aContext->VSSetShaderResources(texture.GetShaderSlot(), 1, texture.GetTextureResource()->Get().GetSRVPtr());
			aContext->PSSetShaderResources(texture.GetShaderSlot(), 1, texture.GetTextureResource()->Get().GetSRVPtr());
		}
	}

	{
		MaterialData& matData = myMaterialBuffer.Map(*aContext);
		matData.myEmissionIntensity = myDesc.myEmissionIntensity;
		myMaterialBuffer.Unmap(*aContext);

		myMaterialBuffer.BindPS(*aContext, ShaderConstants::MaterialConstantBufferSlot);
		myMaterialBuffer.BindVS(*aContext, ShaderConstants::MaterialConstantBufferSlot);
	}

	return effectState;
}

void Engine::EffectPass::SetFloat4(const std::string& aVariableName, const C::Vector4f& aValue)
{
	for (auto& [name, constantBuffer] : myConstantBuffers)
	{
		uint8_t* pointerToVariable = constantBuffer->TryGetVariable(aVariableName);

		if (pointerToVariable)
		{
			*reinterpret_cast<C::Vector4f*>(pointerToVariable) = aValue;
			return;
		}
	}
}

C::Vector4f* Engine::EffectPass::GetFloat4(const std::string& aVariableName)
{
	for (auto& [name, constantBuffer] : myConstantBuffers)
	{
		uint8_t* pointerToVariable = constantBuffer->TryGetVariable(aVariableName);

		if (pointerToVariable)
		{
			return reinterpret_cast<C::Vector4f*>(pointerToVariable);
		}
	}

	return nullptr;
}

void Engine::EffectPass::MapConstantBuffersDataToGPU(ID3D11DeviceContext& aContext) const
{
	for (auto& cbuffer : myConstantBuffers)
	{
		cbuffer.second->MapConstantBufferDataToGPU(aContext);
	}
}

std::unordered_map<std::string, Owned<Engine::EffectConstantBuffer>>& Engine::EffectPass::GetConstantBuffers()
{
	return myConstantBuffers;
}

const std::vector<Engine::EffectTexture>& Engine::EffectPass::GetTextures()
{
	return myTextures;
}

void Engine::EffectPass::CreateConstantBuffersFromShader(ID3D11ShaderReflection* aReflection, ID3D11Device* aDevice, const ShaderFlags aShaderFlag)
{
	for (const auto& loaderConstantBuffer : myDesc.myConstantBuffers)
	{
		const auto findResult = myConstantBuffers.find(myDesc.myName);

		const bool exists = findResult != myConstantBuffers.end();

		if (exists)
		{
			// If found, another shader has already created it
			findResult->second->MakeUsedByShader(aShaderFlag);

			// Skip if exists
			continue;
		}

		auto cbuffer = MakeOwned<EffectConstantBuffer>();

		if (!cbuffer->Init(loaderConstantBuffer, aDevice, aReflection))
		{
			// ERROR_LOG("Failed to init effect constant buffer %s", loaderConstantBuffer.myName.c_str());

			// This shader pixel/vertex did not contain the buffer don't skip it for this shader
			continue;
		}

		cbuffer->MakeUsedByShader(aShaderFlag);

		// Create the constant buffer from the json
		myConstantBuffers.insert(std::make_pair(myDesc.myName, std::move(cbuffer)));
	}
}

void Engine::EffectPass::CreateTexturesFromReflection(ResourceManager& aResourceManager, ID3D11ShaderReflection* aReflection, ID3D11Device* aDevice)
{
	for (const auto& loaderTexture : myDesc.myTextures)
	{
		const auto findResult = std::find_if(myTextures.begin(), myTextures.end(),
			[&loaderTexture](const EffectTexture& aEffectTexture)
			{
				return aEffectTexture.GetNameInShader() == loaderTexture.myNameInShader;
			});

		const bool exists = findResult != myTextures.end();

		if (exists)
		{
			// Skip if exists
			continue;
		}

		EffectTexture effectTexture;

		if (!effectTexture.Init(aResourceManager, loaderTexture, *aDevice, *aReflection))
		{
			// ERROR_LOG("Failed to init effect constant buffer %s", loaderConstantBuffer.myName.c_str());

			// This shader pixel/vertex did not contain the buffer don't skip it for this shader
			continue;
		}

		myTextures.push_back(effectTexture);
	}

	// If they are written in bad order in HLSL sort them here to be 
	// able to use ONE call to set them to the pipeline

	std::sort(myTextures.begin(), myTextures.end(),
		[](const EffectTexture& aLeft, EffectTexture& aRight)
		{
			return aLeft.GetShaderSlot() < aRight.GetShaderSlot();
		});
}

void Engine::EffectPass::ParseReflection(ResourceManager& aResourceManager, ID3D11Device* aDevice, ID3D11ShaderReflection& aReflection, const ShaderFlags aShaderFlag)
{
	// Try to create for pixel shader
	CreateConstantBuffersFromShader(&aReflection, aDevice, aShaderFlag);

	CreateTexturesFromReflection(aResourceManager, &aReflection, aDevice);
}

void Engine::EffectPass::Validate()
{
	std::string texturesFoundString;

	// Ensure that the textures are in a register sequence right 
	// after each other to be able to Bind them using one drawcalls
	if (!myTextures.empty())
	{
		int shaderSlot = myTextures.front().GetShaderSlot();

		for (const auto& textureEffect : myTextures)
		{
			if (textureEffect.GetShaderSlot() != shaderSlot)
			{
				LOG_WARNING(LogType::Renderer) << "The Texture \"" << textureEffect.GetNameInShader() <<
					"\" is not right after the previous texture in the register slot sequence. Make "
					"sure the Texture2D's register slots are e.g. 20, 21, 22 "
					"and not 20, 22, 21. The found texture were: " << texturesFoundString;

				break;
			}

			texturesFoundString += "\"" + textureEffect.GetNameInShader() + "\"(" +
				std::to_string(textureEffect.GetShaderSlot()) + "), ";

			shaderSlot++;
		}
	}
}

Engine::RendererType Engine::EffectPass::GetRendererType() const
{
	return myDesc.myRenderingType;
}

void Engine::EffectPass::OnResourceReloaded()
{
	// When vertex or pixel shader were reloaded
	InitFromLoaderEffectPass(myContext, myDesc);
}

bool Engine::EffectPass::IsInstanced() const
{
	return myIsInstanced;
}

uint16_t Engine::EffectPass::GetID() const
{
	return myID;
}

float Engine::EffectPass::GetEmissionIntensity() const
{
	return myDesc.myEmissionIntensity;
}

void Engine::EffectPass::CreateMaterialConstantBuffer(MaterialFactoryContext& aFactoryContext)
{
	if (!myMaterialBuffer.InitDynamicWritable())
	{
		LOG_ERROR(LogType::Renderer) << "CreateBuffer failed to create material cbuffer";
	}
}

//size_t Renderer::EffectPass::GetHash() const
//{
//	return myHash;
//}
//
//void Renderer::EffectPass::UpdateHash()
//{
//	// MaterialPath + Pass Index
//	std::hash<std::string> hasher;
//	myHash = hasher(myMaterialName + std::to_string(myEffectIndex));
//}