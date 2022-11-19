#pragma once

#include "Engine\Renderer\Texture\FullscreenTexture.h"

namespace Engine
{
	class FullscreenTextureFactory;
	class WindowHandler;
	struct BloomSettings2;
	class FullscreenRenderer;

	class Bloom
	{
	public:
		Bloom(
			FullscreenTextureFactory& aFullscreenTextureFactory,
			WindowHandler& aWindowHandler);
		~Bloom();

		bool Init(ID3D11Device& aDevice);

		void CreateTextures();
		void ClearTextures();

		void Render(
			const BloomSettings2& aBloomSettings, 
			ID3D11DeviceContext& aContext,
			FullscreenRenderer& aFullscreenRenderer,
			FullscreenTexture& aMainTexture,
			FullscreenTexture*& aCurrentResultTexture);

	private:
		void CreateConstantBuffers(ID3D11Device& aDevice);

	private:
		FullscreenTextureFactory* myFullscreenTextureFactory = nullptr;
		WindowHandler* myWindowHandler = nullptr;

		struct BloomData
		{
			Vec4f myParams; // x: pass, y: intensity, z: samplescale, w: 0
			Vec4f myThreshold; // x: threshold, y: threshold - knee, z: knee * 2, z: 0.25 / knee
		} myBloomData = { };

		int myBloomIterations = 4;

		ID3D11Buffer* myBloomDataBuffer = nullptr;
		ID3D11Buffer* myBloomPassBuffer = nullptr;

		std::vector<FullscreenTexture> myBloomTextures;
		std::vector<FullscreenTexture> myBloomTexturesUpsample;
		FullscreenTexture myBloomFinalResultTexture;
	};
}