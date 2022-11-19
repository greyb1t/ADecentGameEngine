#include "pch.h"
#include "Bloom.h"
#include "Engine\Renderer\Texture\FullscreenTexture.h"
#include "Engine\Renderer\WindowHandler.h"
#include "Engine\Renderer\Texture\FullscreenTextureFactory.h"
#include "BloomSettings.h"
#include "Engine\Renderer\Utils\DxUtils.h"
#include "Engine\Renderer\PostProcessing\FullscreenRenderer.h"

int GetMipLevelCount(int aWidth, int aHeight)
{
	// TODO(filip): must add +1 to get correct result
	// also, bloom texture sizes and shit like that is wrong I think
	// refactor and make cleaner shit in term of easier calculating the
	// bloom texture sizes
	return static_cast<int>(std::floor(log2(std::min(aWidth, aHeight))))/* + 1*/;
}

Engine::Bloom::Bloom(
	FullscreenTextureFactory& aFullscreenTextureFactory,
	WindowHandler& aWindowHandler)
	: myFullscreenTextureFactory(&aFullscreenTextureFactory),
	myWindowHandler(&aWindowHandler)
{
}

Engine::Bloom::~Bloom()
{
}

bool Engine::Bloom::Init(ID3D11Device& aDevice)
{
	CreateConstantBuffers(aDevice);
	CreateTextures();

	return true;
}

void Engine::Bloom::CreateTextures()
{
	const auto renderSizeui = myWindowHandler->GetRenderingSize();

	const int iterations = GetMipLevelCount(renderSizeui.x, renderSizeui.y);
	assert(iterations < 16 && "is it ok with more than 16? or clamp?");

	myBloomTextures.clear();

	// Create each bloom textures, must be scalefactor = 0.5^n
	// These textures are only used in the downsample steps
	for (int i = 0; i < iterations; ++i)
	{
		const float scaleFactor = std::pow(0.5f, static_cast<float>(i + 1));

		myBloomTextures.push_back(FullscreenTexture(
			myFullscreenTextureFactory->CreateTexture(
				(renderSizeui.CastTo<float>() * scaleFactor).CastTo<unsigned int>(),
				DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT)));
	}

	myBloomTexturesUpsample.clear();
	for (int i = 0; i < iterations; ++i)
	{
		const float scaleFactor = std::pow(0.5f, static_cast<float>(i + 1));

		myBloomTexturesUpsample.push_back(FullscreenTexture(
			myFullscreenTextureFactory->CreateTexture(
				(renderSizeui.CastTo<float>() * scaleFactor).CastTo<unsigned int>(),
				DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT)));
	}


	myBloomFinalResultTexture = FullscreenTexture(
		myFullscreenTextureFactory->CreateTexture(
			renderSizeui,
			DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT));
}

void Engine::Bloom::ClearTextures()
{
	myBloomFinalResultTexture.ClearTexture();

	for (auto& texture : myBloomTextures)
	{
		texture.ClearTexture();
	}

	for (auto& texture : myBloomTexturesUpsample)
	{
		texture.ClearTexture();
	}
}

void Engine::Bloom::Render(
	const BloomSettings2& aBloomSettings,
	ID3D11DeviceContext& aContext,
	FullscreenRenderer& aFullscreenRenderer,
	FullscreenTexture& aMainTexture,
	FullscreenTexture*& aCurrentResultTexture)
{
	// Bind the bloom settings
	{
		myBloomData.myParams = {};
		myBloomData.myParams.y = aBloomSettings.myIntensity;
		myBloomData.myParams.z = aBloomSettings.mySampleScale;

		myBloomData.myThreshold.x = aBloomSettings.myThreshold;

		float knee = aBloomSettings.myThreshold * aBloomSettings.mySoftThreshold + 0.0001f;

		myBloomData.myThreshold.y = aBloomSettings.myThreshold - knee;
		myBloomData.myThreshold.z = knee * 2.f;
		myBloomData.myThreshold.w = 0.25f / knee;

		D3D11_MAPPED_SUBRESOURCE mappedData = {};
		HRESULT result = aContext.Map(myBloomDataBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
		ThrowIfFailed(result);

		memcpy(mappedData.pData, &myBloomData, sizeof(myBloomData));

		aContext.Unmap(myBloomDataBuffer, 0);
	}

	// 
	// SetSamplerState(0, SamplerState_Trilinear);

	enum class BloomPass
	{
		Prefilter = 0,
		DownSample = 1,
		Upsample = 2
	};

#if 1
	// We work on half res texture, because accoriding to unity, working on full res does not give much and kills performance on fill-rate dependant platforms
	const auto bloomTextureSize = myWindowHandler->GetRenderingSize() / 2u;

	int iterations = GetMipLevelCount(bloomTextureSize.x, bloomTextureSize.y);
	assert(iterations < 16 && "is it ok with more than 16? or clamp?");

	// vi generera för fan inte mipmaps, och använder SampleLevel
	// wack ju

	// Doing it to max mip becomes 1 pixel lol?
	// therefore only from 0 - 4 or something atm
	// TODO: Make this value adjustable as a setting
	iterations -= Math::Clamp(aBloomSettings.myIterationsOffset, 0, iterations - 1);

	// Downsample
	for (int mipLevel = 0; mipLevel < iterations; ++mipLevel)
	{
		const int pass = mipLevel == 0
			? static_cast<int>(BloomPass::Prefilter)
			: static_cast<int>(BloomPass::DownSample);

		{
			myBloomData.myParams.x = pass; // pass

			D3D11_MAPPED_SUBRESOURCE mappedData = {};
			HRESULT result = aContext.Map(myBloomDataBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
			ThrowIfFailed(result);

			memcpy(mappedData.pData, &myBloomData, sizeof(myBloomData));

			aContext.Unmap(myBloomDataBuffer, 0);
		}

		aContext.PSSetConstantBuffers(4, 1, &myBloomDataBuffer);

		FullscreenTexture* texture = nullptr;

		texture = &myBloomTextures[mipLevel];

		texture->SetAsActiveTarget();
		aCurrentResultTexture->SetAsResourceOnSlot(0);
		aFullscreenRenderer.Render(FullscreenRenderer::Shader::Bloom2);

		// The next iteration, we downsample the already downsampled texture
		aCurrentResultTexture = texture;
	}

	/*
		Upsample pass:

		In the previous pass we created our own mip map levels of the main texture

		We now go from the highest mip map (smallest texture) down to
		mip map 0 and for each iteration upsample and combine pixels additively.
	*/

	// myCurrentResultTexture is already the largest mip map level
	// therefore simply continue using it below

	// First iteration:
	// Texture0: 2nd largest mip map level
	// Texture1: largest mip map level
	// 
	// Upsample Texture1 and add additively onto Texture0.
	// Render to Texture0
	// 
	// and continue iterations...

	// This is the texture we upsample and then add onto the current mip
	FullscreenTexture* previousUpperMip = &myBloomTextures[iterations - 1];

	// -1 and -1 so we begin at the 2nd largest mip map because we cannot
	// combine the largest one because no higher mip map level to combine with
	for (int mipLevel = iterations - 1 - 1; mipLevel >= 0; --mipLevel)
	{
		{
			myBloomData.myParams.x = static_cast<int>(BloomPass::Upsample);

			D3D11_MAPPED_SUBRESOURCE mappedData = {};
			HRESULT result = aContext.Map(myBloomDataBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
			ThrowIfFailed(result);

			memcpy(mappedData.pData, &myBloomData, sizeof(myBloomData));

			aContext.Unmap(myBloomDataBuffer, 0);
		}

		aContext.PSSetConstantBuffers(4, 1, &myBloomDataBuffer);

		FullscreenTexture* texture = nullptr;

		texture = &myBloomTexturesUpsample[mipLevel];

		texture->SetAsActiveTarget();

		// Set the 1-step larger mip level as resource
		// Texture1
		// Main texture is the texture we want to upsample
		previousUpperMip->SetAsResourceOnSlot(0);

		// Set current mip level
		// Texture0
		myBloomTextures[mipLevel].SetAsResourceOnSlot(1);

		//prevTexture->SetAsResourceOnSlot(1);
		// assert(false);
		aFullscreenRenderer.Render(FullscreenRenderer::Shader::Bloom2);

		previousUpperMip = texture;
	}

	// Must unbind to use the previousUpperMip texture
	DxUtils::UnbindRenderTargetView(aContext);

	// TODO: LAST STEP IS USE OLD BLOOM SHADER TO ADDITIVELY ADD THE
	// MAIN RESULT WITH THE FINAL BLOOM RESULT

	aMainTexture.SetAsResourceOnSlot(0);
	previousUpperMip->SetAsResourceOnSlot(1);
	myBloomFinalResultTexture.SetAsActiveTarget();
	aFullscreenRenderer.Render(FullscreenRenderer::Shader::Bloom);

	aCurrentResultTexture = &myBloomFinalResultTexture;
#endif
}

void Engine::Bloom::CreateConstantBuffers(ID3D11Device& aDevice)
{
	D3D11_BUFFER_DESC bufDesc = {};
	{
		bufDesc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
		bufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}

	bufDesc.ByteWidth = sizeof(BloomData);

	HRESULT result = aDevice.CreateBuffer(&bufDesc, nullptr, &myBloomDataBuffer);

	if (FAILED(result))
	{
		LOG_ERROR(LogType::Renderer) << "CreateBuffer failed to create frame cbuffer";
	}
}
