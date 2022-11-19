#pragma once

namespace Engine
{
	class FullscreenTexture;

	class GBuffer
	{
		friend class FullscreenTextureFactory;

	public:

		enum GBufferTexture
		{
			Position,
			Albedo,
			Normal,
			VertexNormal,
			Material,
			AmbientOcclusion,
			Depth,
			Count
		};

		GBuffer();
		~GBuffer() = default;

		bool Init(ID3D11DeviceContext* aContext);

		void ClearTextures();
		void SetAsActiveTarget(FullscreenTexture* aDepth = nullptr);
		void SetAsResourceOnSlot(GBufferTexture aTexture, unsigned int aSlot) const;
		void SetAllAsResources();
		void UnbindAllResources();

		ID3D11RenderTargetView* GetRenderTarget(const GBufferTexture aTexture);

	private:
		std::array<ComPtr<ID3D11Texture2D>, GBufferTexture::Count> myTextures = {};
		std::array<ComPtr<ID3D11RenderTargetView>, GBufferTexture::Count> myRTVs = {};
		std::array<ComPtr<ID3D11ShaderResourceView>, GBufferTexture::Count> mySRVs = {};
		D3D11_VIEWPORT* myViewport = nullptr;

		ID3D11DeviceContext* myContext = nullptr;
	};
}