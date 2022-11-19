#pragma once

#include "Engine/Renderer/Texture/FullscreenTexture.h"
#include "GBuffer.h"
#include "RenderEnums.h"
#include "SharedRendererData.h"
#include "Utils/Observer.h"
#include "Statistics/RenderingStatistics.h"

namespace Engine
{
	class ResourceManager;
	class ResourceReferences;
	class Scene;
}

namespace Engine
{
	class SSAO;
	class RenderCommandList;
	class RendererScene;
	class Directx11Framework;
	class FullscreenTextureFactory;
	class CascadedShadows;
	class GraphicsEngine;
	class WindowHandler;
	struct Times;
	class FullscreenEffectManager;
	class DeferredRenderer;
	class DebugRenderer;
	class ParticleRenderer;
	class SpriteRenderer;
	class ShadowRenderer;
	class TextRenderer;
	class ForwardRenderer;
	class FullscreenRenderer;
	class FrameBufferTempName;
	class Camera;
	struct CulledSceneData;
	class SelectionRenderer;

	class RenderManager : public Observer
	{
	public:
		RenderManager(
			ResourceManager& aResourceManager,
			ResourceReferences& aResourceRefs,
			FullscreenTextureFactory& aFullscreenTextureFactory, 
			WindowHandler& aWindowHandler, 
			Directx11Framework& aFramework);
		~RenderManager();

		RenderManager(const RenderManager&) = delete;

		bool Init(Directx11Framework& aFramework);
		// ScenesRenderingStats Render(RendererScene& aScene, RendererScene* aUIScene);
		void EndRender(RendererScene& aScene);
		void EndRender2();
		RenderingStatistics RenderScene(
			RendererScene& aScene, 
			const Camera& aCamera, 
			const bool aUI,
			const CulledSceneData& aCulledSceneData);
		void DrawDebugMenu();

		void SetBlendState(const BlendState aBlendState);
		void SetDepthStencilState(const DepthStencilState aDepthStencil);
		void SetSamplerState(const int aSlot, const SamplerState aSamplerState);
		void SetRasterizerState(const RasterizerState aRasterizeState);

		ID3D11DepthStencilState* GetDepthStencilState(const DepthStencilState aDepthStencilState);
		ID3D11RasterizerState* GetRasterizerState(const RasterizerState aRasterizerState);
		ID3D11BlendState* GetBlendState(const BlendState aBlendState);

		const ForwardRenderer &GetForwardRenderer() const;
		const FullscreenRenderer &GetFullscreenRenderer() const;
		const DeferredRenderer& GetDeferredRenderer() const;
		const ParticleRenderer &GetParticleRenderer() const;
		const SpriteRenderer& GetSpriteRenderer() const;
		const TextRenderer& GetTextRenderer() const;
		DebugRenderer& GetDebugRenderer();

		void Receive(const EventType aEventType, const std::any& aValue) override;

		FrameBufferTempName& GetFrameBuffer();

		void RenderSceneToBackBuffer(Scene& aScene);
		void RenderResultToBackBuffer(Scene& aScene);

		ID3D11DepthStencilView* GetReadOnlyDepthStencilView();

	private:
		bool InitBlendStates(Directx11Framework& aFramework);
		bool InitDepthStencilStates(Directx11Framework& aFramework);
		bool InitRasterizerStates(Directx11Framework& aFramework);
		bool InitSamplerStates(Directx11Framework& aFramework);

		void RenderToBackbuffer(FullscreenTexture& aTexture);

		void OnResized(const WindowResizedData& aResizeData);

		void CreateTextures();
		void DepthPrePass(
			const RenderCommandList& aRenderCommandChain, 
			FullscreenTexture& aDepthTexture);

		void CreateReadOnlyDepthStencilView(FullscreenTexture& aDepthTexture);

	private:
		ResourceManager& myResourceManager;
		ResourceReferences& myResourceRefs;

		FullscreenTextureFactory& myFullscreenTextureFactory;
		WindowHandler& myWindowHandler;
		Directx11Framework& myFramework;

		Owned<ForwardRenderer> myForwardRenderer;
		Owned<FullscreenRenderer> myFullscreenRenderer;
		Owned<DeferredRenderer> myDeferredRenderer;
		Owned<DebugRenderer> myDebugRenderer;
		Owned<ParticleRenderer> myParticleRenderer;
		Owned<SpriteRenderer> mySpriteRenderer;
		Owned<TextRenderer> myTextRenderer;
		Owned<ShadowRenderer> myShadowRenderer;
		Owned<SelectionRenderer> mySelectionRenderer;

		FullscreenTexture myDepthTexture;

		GBuffer myGBuffer;
		FullscreenTexture myDeferredTexture;
		FullscreenTexture myDeferredTexture2; // only used as output from the post processing
		FullscreenTexture myTempFogTexture;
		FullscreenTexture myDebugResultTexture;

		FullscreenTexture myDepthBufferCopy;
		FullscreenTexture myDepthBufferCopy2;

		FullscreenTexture mySelectionTexture;
		FullscreenTexture mySelectionDepthTexture;

		ShaderRendererData mySharedRendererData;

		Owned<FrameBufferTempName> myFrameBuffer;

		Owned<SSAO> mySSAO;

		ComPtr<ID3D11DepthStencilView> myReadOnlyDepthStencilView;

		std::array<ID3D11BlendState*, BlendState_Count> myBlendStates = {};
		std::array<ID3D11DepthStencilState*, DepthStencilState_Count> myDepthStencilStates = {};
		std::array<ID3D11RasterizerState*, RasterizerState_Count> myRasterizerStates = {};
		std::array<ID3D11SamplerState*, SamplerState_Count> mySamplerStates = {};
	};
}