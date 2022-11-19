#include "pch.h"
#include "FullscreenRenderer.h"
#include "..\ResourceManagement\Resources\VertexShaderResource.h"
#include "..\ResourceManagement\Resources\PixelShaderResource.h"
#include "Engine/Renderer/Shaders/PixelShader.h"
#include "Engine/Renderer/Shaders/VertexShader.h"
#include "..\ResourceManagement\ResourceManager.h"
#include "Engine\Renderer\Texture\FullscreenTexture.h"
#include "..\Utils\DxUtils.h"
#include "Engine\Renderer\GBuffer.h"
#include "PostProcessData.h"
#include "Engine\Renderer\Texture\FullscreenTextureFactory.h"
#include "Engine\Renderer\WindowHandler.h"
#include "Engine\Renderer\Directx11Framework.h"
#include "Engine\Renderer\TracyProfilingGPU.h"
#include "Bloom/Bloom.h"
#include "SSAO\SSAO.h"
#include "Engine\Engine.h"
#include "Engine\Paths.h"

namespace Engine
{
	FullscreenRenderer::FullscreenRenderer(
		ResourceManager& aResourceManager,
		FullscreenTextureFactory& aFullscreenTextureFactory,
		WindowHandler& aWindowHandler)
		: myResourceManager(aResourceManager),
		myFullscreenTextureFactory(&aFullscreenTextureFactory),
		myWindowHandler(&aWindowHandler)
	{
	}

	FullscreenRenderer::~FullscreenRenderer()
	{
	}

	bool FullscreenRenderer::Init(ID3D11Device* aDevice, ID3D11DeviceContext* aContext)
	{
		myContext = aContext;
		myDevice = aDevice;

		myVertexShader =
			myResourceManager.CreateRef<VertexShaderResource>(
				"Assets/Shaders/Engine/Fullscreen/FullscreenVS",
				nullptr,
				0);
		myVertexShader->RequestLoading();

		std::array<std::string, static_cast<int>(Shader::Count)> shaderPaths;
		shaderPaths[static_cast<int>(Shader::Copy)] = "Assets/Shaders/Engine/Fullscreen/FullscreenPS-Copy";

		shaderPaths[static_cast<int>(Shader::CopyR)] = "Assets/Shaders/Engine/Fullscreen/FullscreenPS-CopyR";
		shaderPaths[static_cast<int>(Shader::CopyG)] = "Assets/Shaders/Engine/Fullscreen/FullscreenPS-CopyG";
		shaderPaths[static_cast<int>(Shader::CopyB)] = "Assets/Shaders/Engine/Fullscreen/FullscreenPS-CopyB";
		shaderPaths[static_cast<int>(Shader::CopyA)] = "Assets/Shaders/Engine/Fullscreen/FullscreenPS-CopyA";

		shaderPaths[static_cast<int>(Shader::Luminance)] = "Assets/Shaders/Engine/Fullscreen/FullscreenPS-Luminance";
		shaderPaths[static_cast<int>(Shader::GaussianHorizontal)] = "Assets/Shaders/Engine/Fullscreen/FullscreenPS-GaussianH";
		shaderPaths[static_cast<int>(Shader::GaussianVertical)] = "Assets/Shaders/Engine/Fullscreen/FullscreenPS-GaussianV";
		shaderPaths[static_cast<int>(Shader::Bloom)] = "Assets/Shaders/Engine/Fullscreen/FullscreenPS-Bloom";
		shaderPaths[static_cast<int>(Shader::Bloom2)] = "Assets/Shaders/Engine/Fullscreen/FullscreenPS-Bloom2";
		shaderPaths[static_cast<int>(Shader::Fog)] = "Assets/Shaders/Engine/Fullscreen/FullscreenPS-Fog";
		shaderPaths[static_cast<int>(Shader::GammaCorrection)] = "Assets/Shaders/Engine/Fullscreen/FullscreenPS-GammaCorrection";
		shaderPaths[static_cast<int>(Shader::CopyDepth)] = "Assets/Shaders/Engine/Fullscreen/FullscreenPS-CopyDepth";
		shaderPaths[static_cast<int>(Shader::FXAA)] = "Assets/Shaders/Engine/Fullscreen/FullscreenPS-FXAA";
		shaderPaths[static_cast<int>(Shader::Tonemapping)] = "Assets/Shaders/Engine/Fullscreen/FullscreenPS-Tonemapping";
		shaderPaths[static_cast<int>(Shader::SSAO)] = "Assets/Shaders/Engine/Fullscreen/FullscreenPS-SSAO";
		shaderPaths[static_cast<int>(Shader::SSAOBlur)] = "Assets/Shaders/Engine/Fullscreen/FullscreenPS-SSAOBlur";

		for (unsigned int i = 0; i < static_cast<unsigned int>(Shader::Count); ++i)
		{
			myPixelShaders[i] =
				myResourceManager.CreateRef<PixelShaderResource>(shaderPaths[i]);
			myPixelShaders[i]->RequestLoading();
		}

		CreateTextures();

		CreateConstantBuffers(*aDevice);

		return true;
	}

	void FullscreenRenderer::Render(const Shader aMode)
	{
		myContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		myContext->IASetInputLayout(nullptr);
		myContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
		myContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);

		myContext->VSSetShader(myVertexShader->Get().GetVertexShader(), nullptr, 0);
		myContext->PSSetShader(myPixelShaders[static_cast<int>(aMode)]->Get().GetPixelShader(), nullptr, 0);
		myContext->Draw(3, 0);
	}

	void FullscreenRenderer::RenderPostProcess(
		const PostProcessData& aPostProcessData,
		FullscreenTexture& aMainTexture,
		FullscreenTexture& aMainDepthTexture,
		GBuffer& aGBufferTexture,
		FullscreenTexture& aResultTexture)
	{
		GPU_ZONE_NAMED(postProcessZone, "Post Processing");

		myCurrentResultTexture = &aMainTexture;

		// Post process order: https://gamedev.stackexchange.com/questions/147952/what-is-the-order-of-postprocessing-effects

		// Bloom
		if (aPostProcessData.myBloomSettings.myEnabled)
		{
			GPU_ZONE_NAMED(bloomZone, "Bloom");

			if (myBloom == nullptr)
			{
				myBloom = MakeOwned<Bloom>(
					*myFullscreenTextureFactory,
					*myWindowHandler);

				myBloom->Init(*myDevice);
			}

			myBloom->Render(
				aPostProcessData.myBloomSettings,
				*myContext,
				*this,
				aMainTexture,
				myCurrentResultTexture);
		}
		else
		{
			myBloom = nullptr;
		}

		// Tone mapping
		if (aPostProcessData.myTonemappingData.myEnabled)
		{
			GPU_ZONE_NAMED(tonemappingZone, "Tonemapping");

			// Set the tonemap data
			{
				myTonemapData.myTonemapType = aPostProcessData.myTonemappingData.myTonemapType.GetValue();
				myTonemapData.myExposure = aPostProcessData.myTonemappingData.myExposure;

				D3D11_MAPPED_SUBRESOURCE mappedData = { };
				HRESULT result = myContext->Map(myTonemapDataBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
				ThrowIfFailed(result);

				memcpy(mappedData.pData, &myTonemapData, sizeof(myTonemapData));

				myContext->Unmap(myTonemapDataBuffer, 0);
			}

			myContext->PSSetConstantBuffers(4, 1, &myTonemapDataBuffer);

			myTonemappingTexture.SetAsActiveTarget();
			myCurrentResultTexture->SetAsResourceOnSlot(0);
			Render(Shader::Tonemapping);

			myCurrentResultTexture = &myTonemappingTexture;
		}

		// FXAA
		if (aPostProcessData.myFXAASettings.myEnabled)
		{
			GPU_ZONE_NAMED(FXAAZone, "FXAA");

			myFXAATexture.SetAsActiveTarget();
			myCurrentResultTexture->SetAsResourceOnSlot(0);
			Render(Shader::FXAA);

			myCurrentResultTexture = &myFXAATexture;
		}

		aResultTexture.SetAsActiveTarget();
		myCurrentResultTexture->SetAsResourceOnSlot(0);
		Render(Shader::Copy);
	}

	VertexShaderRef FullscreenRenderer::GetFullscreenVertexShader()
	{
		return myVertexShader;
	}

	void FullscreenRenderer::ClearTextures()
	{
		if (myBloom)
		{
			myBloom->ClearTextures();
		}
	}

	void FullscreenRenderer::OnResized()
	{
		CreateTextures();
	}

	void FullscreenRenderer::CreateTextures()
	{
		const auto renderSizeui = myWindowHandler->GetRenderingSize();

		myFXAATexture = FullscreenTexture(
			myFullscreenTextureFactory->CreateTexture(
				renderSizeui,
				DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT));

		myTonemappingTexture = FullscreenTexture(
			myFullscreenTextureFactory->CreateTexture(
				renderSizeui,
				DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT));

		if (myBloom)
		{
			myBloom->CreateTextures();
		}
	}

	void FullscreenRenderer::CreateConstantBuffers(ID3D11Device& aDevice)
	{
		D3D11_BUFFER_DESC bufDesc = { };
		{
			bufDesc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
			bufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		}

		bufDesc.ByteWidth = sizeof(TonemappingData);

		HRESULT result = aDevice.CreateBuffer(&bufDesc, nullptr, &myTonemapDataBuffer);

		if (FAILED(result))
		{
			LOG_ERROR(LogType::Renderer) << "CreateBuffer failed to create frame cbuffer";
		}
	}

}