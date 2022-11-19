#pragma once

#include "Engine\Renderer\Texture\FullscreenTexture.h"
#include "Engine\Renderer\ConstantBuffer.h"

namespace Engine
{
	class FullscreenTextureFactory;
	class WindowHandler;
	class GBuffer;
	class FullscreenRenderer;

	class SSAO
	{
	public:
		SSAO(
			FullscreenTextureFactory& aFullscreenTextureFactory,
			WindowHandler& aWindowHandler);
		~SSAO();

		bool Init(ID3D11Device& aDevice);

		void Render(
			const GBuffer& aGBuffer,
			ID3D11DeviceContext& aContext,
			FullscreenRenderer& aFullscreenRenderer);

		void ClearTextures();
		void CreateTextures();

		FullscreenTexture& GetResultTexture();
		FullscreenTexture& GetResultTextureBlurred();

	private:
		void GenerateSampleKernel();
		std::vector<Vec4f> CreateRotationVectors();

	private:
		FullscreenTextureFactory* myFullscreenTextureFactory = nullptr;
		WindowHandler* myWindowHandler = nullptr;

		FullscreenTexture myNoiseTexture;

		FullscreenTexture myResultTexture;

		FullscreenTexture myResultTextureBlurred;

		struct SSAOData
		{
			Vec4f mySamples[16];
			Vec2f mySSAOTextureResolution;
			Vec2f myNoiseTextureScale;
		};

		std::vector<Vec3f> ssaoKernel;

		ConstantBuffer<SSAOData> mySSAOBuffer;
	};
}