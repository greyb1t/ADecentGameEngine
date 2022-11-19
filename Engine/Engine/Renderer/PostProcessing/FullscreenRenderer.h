#pragma once

#include "..\ResourceManagement\ResourceRef.h"
#include "Engine\Renderer\Texture\FullscreenTexture.h"

namespace Engine
{
	class Bloom;
	class SSAO;
	class ResourceManager;
	class FullscreenTexture;
	class GBuffer;
	struct PostProcessData;
	class FullscreenTextureFactory;
	class WindowHandler;

	class FullscreenRenderer
	{
	public:
		enum class Shader
		{
			Copy,

			// Only used to debug gbuffer textures
			CopyR,
			CopyG,
			CopyB,
			CopyA,

			Luminance,
			GaussianHorizontal,
			GaussianVertical,
			Bloom,
			Fog,
			GammaCorrection,
			CopyDepth,
			FXAA,
			Tonemapping,
			Bloom2,
			SSAO,
			SSAOBlur,

			Count
		};

		FullscreenRenderer(
			ResourceManager& aResourceManager,
			FullscreenTextureFactory& aFullscreenTextureFactory,
			WindowHandler& aWindowHandler);

		~FullscreenRenderer();

		bool Init(ID3D11Device* aDevice, ID3D11DeviceContext* aContext);
		void Render(const Shader aMode);

		void RenderPostProcess(
			const PostProcessData& aPostProcessData,
			FullscreenTexture& aMainTexture,
			FullscreenTexture& aMainDepthTexture,
			GBuffer& aGBufferTexture,
			FullscreenTexture& aResultTexture);

		VertexShaderRef GetFullscreenVertexShader();

		void ClearTextures();

		void OnResized();
		void CreateTextures();
		void CreateConstantBuffers(ID3D11Device& aDevice);

	private:
		ResourceManager& myResourceManager;
		FullscreenTextureFactory* myFullscreenTextureFactory = nullptr;
		WindowHandler* myWindowHandler = nullptr;

		VertexShaderRef myVertexShader;
		std::array<PixelShaderRef, static_cast<int>(Shader::Count)> myPixelShaders = { };

		ID3D11DeviceContext* myContext = nullptr;
		ID3D11Device* myDevice = nullptr;

		Owned<Bloom> myBloom;

		FullscreenTexture* myCurrentResultTexture = nullptr;

		FullscreenTexture myFXAATexture;
		FullscreenTexture myTonemappingTexture;

		struct TonemappingData
		{
			int myTonemapType = 0;
			float myExposure = 0.f;
			Vec2f myTrash_;
		} myTonemapData;

		ID3D11Buffer* myTonemapDataBuffer = nullptr;
	};
}