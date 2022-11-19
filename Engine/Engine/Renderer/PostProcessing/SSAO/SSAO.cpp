#include "pch.h"
#include "SSAO.h"
#include "Engine\Renderer\GBuffer.h"
#include "Engine\Renderer\Texture\FullscreenTextureFactory.h"
#include "Engine\Renderer\WindowHandler.h"
#include "Engine\Renderer\PostProcessing\FullscreenRenderer.h"

Engine::SSAO::SSAO(
	FullscreenTextureFactory& aFullscreenTextureFactory,
	WindowHandler& aWindowHandler)
	: myFullscreenTextureFactory(&aFullscreenTextureFactory),
	myWindowHandler(&aWindowHandler)
{

}

Engine::SSAO::~SSAO()
{

}

bool Engine::SSAO::Init(ID3D11Device& aDevice)
{
	GenerateSampleKernel();

	if (!mySSAOBuffer.InitDynamicWritable())
	{
		return false;
	}

	CreateTextures();

	return true;
}

void Engine::SSAO::Render(
	const GBuffer& aGBuffer,
	ID3D11DeviceContext& aContext,
	FullscreenRenderer& aFullscreenRenderer)
{
	// TODO: Set the sampler to repeat for 4x4 rotation vector texture
	// is that wrap? D3D11_TEXTURE_ADDRESS_WRAP
	// Create a WRAP sampler state that is trilinear, the same we read normal gbuffer position deferred shaders

	myResultTexture.ClearTexture();
	myResultTexture.SetAsActiveTarget();

	aGBuffer.SetAsResourceOnSlot(GBuffer::GBufferTexture::Position, 5);
	aGBuffer.SetAsResourceOnSlot(GBuffer::GBufferTexture::Normal, 2);
	aGBuffer.SetAsResourceOnSlot(GBuffer::GBufferTexture::VertexNormal, 6);
	// aGBuffer.SetAsResourceOnSlot(GBuffer::GBufferTexture::Albedo, 3);

	myNoiseTexture.SetAsResourceOnSlot(3);

	SSAOData& data = mySSAOBuffer.Map(aContext);

	for (size_t i = 0; i < ssaoKernel.size(); ++i)
	{
		data.mySamples[i] = ssaoKernel[i].ToVec4(0.f);
	}

	const auto renderSizeui = myWindowHandler->GetRenderingSize();
	data.mySSAOTextureResolution = renderSizeui.CastTo<float>();
	data.myNoiseTextureScale = Vec2f(4.f, 4.f);

	mySSAOBuffer.Unmap(aContext);

	aContext.PSSetConstantBuffers(4, 1, mySSAOBuffer.GetAddressOf());

	aFullscreenRenderer.Render(FullscreenRenderer::Shader::SSAO);

	DxUtils::UnbindRenderTargetView(aContext);

	// Blur

	myResultTextureBlurred.ClearTexture();

	myResultTexture.SetAsResourceOnSlot(0);
	myResultTextureBlurred.SetAsActiveTarget();

	aFullscreenRenderer.Render(FullscreenRenderer::Shader::SSAOBlur);

	DxUtils::UnbindRenderTargetView(aContext);
}

void Engine::SSAO::ClearTextures()
{
	myResultTexture.ClearTexture();
	myResultTextureBlurred.ClearTexture();
}

void Engine::SSAO::CreateTextures()
{
	const auto renderSizeui = myWindowHandler->GetRenderingSize();

	const auto noise = CreateRotationVectors();

	// these should always be the same I think
	const int width = 4;
	const int height = 4;

	D3D11_SUBRESOURCE_DATA noiseTextureData = { 0 };
	noiseTextureData.pSysMem = &noise[0];
	noiseTextureData.SysMemPitch = width * sizeof(Vec4f);
	noiseTextureData.SysMemSlicePitch = 0;

	// 4, 4 is based on kernel size?
	// 4 * 4 = 16
	myNoiseTexture = myFullscreenTextureFactory->CreateTexture(
		Vec2ui(width, height),
		DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT,
		&noiseTextureData);

	myResultTexture = myFullscreenTextureFactory->CreateTexture(
		renderSizeui,
		DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT);

	myResultTextureBlurred = myFullscreenTextureFactory->CreateTexture(
		renderSizeui,
		DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT);
}

Engine::FullscreenTexture& Engine::SSAO::GetResultTexture()
{
	return myResultTexture;
}

Engine::FullscreenTexture& Engine::SSAO::GetResultTextureBlurred()
{
	return myResultTextureBlurred;
}

float lerp(float a, float b, float f)
{
	return a + f * (b - a);
}

void Engine::SSAO::GenerateSampleKernel()
{
	std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
	std::default_random_engine generator;

	for (unsigned int i = 0; i < 16; ++i)
	{
		Vec3f sample(
			Random::RandomFloat(-1.f, 1.f)/*randomFloats(generator) * 2.0 - 1.0*/,
			Random::RandomFloat(-1.f, 1.f)/*randomFloats(generator) * 2.0 - 1.0*/,
			randomFloats(generator)
		);
		sample.Normalize();
		sample *= randomFloats(generator);

		float scale = float(i) / 16.0f;

		// scale samples s.t. they're more aligned to center of kernel
		scale = Math::Lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;

		ssaoKernel.push_back(sample);
	}
}

std::vector<Vec4f> Engine::SSAO::CreateRotationVectors()
{
	std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
	std::default_random_engine generator;

	std::vector<Vec4f> ssaoNoise;
	for (unsigned int i = 0; i < 16; i++)
	{
		Vec4f noise(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			0.0f,
			0.0f);
		ssaoNoise.push_back(noise.GetNormalized());
	}

	return ssaoNoise;
}
