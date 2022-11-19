#pragma once

struct ID3D11Texture2D;
struct ID3D11RenderTargetView;
struct ID3D11DeviceContext;
struct ID3D11DepthStencilView;
struct ID3D11ShaderResourceView;
struct D3D11_VIEWPORT;

namespace Engine
{
	class FullscreenTexture;
	class Directx11Framework;

	// Only used because we need to store the data 
	// somewhere for when we need to resize the window
	struct LoaderFullscreenTextures
	{
		std::string myTextureName;
		Vec2f myTextureScale;
	};

	class FullscreenTexture
	{
	public:
		FullscreenTexture();
		FullscreenTexture(ID3D11DeviceContext& aContext);
		~FullscreenTexture();

		void ClearTexture(C::Vector4f aClearColor = { 0.f, 0.f, 0.f, 0.f });
		void ClearDepth(float aClearDepthValue = 1.0f, unsigned int aClearStencilValue = 0);
		void SetAsActiveTarget(FullscreenTexture* aDepth = nullptr);
		void SetAsResourceOnSlot(unsigned int aSlot);
		void SetAsActiveDepth();

		ID3D11Texture2D* GetTextureTest();
		ID3D11RenderTargetView* GetRenderTargetTest();
		ID3D11RenderTargetView** GetRenderTargetTestPtr();
		ID3D11ShaderResourceView* GetSRV();
		ID3D11ShaderResourceView** GetSRVAddressOf();

		ID3D11DepthStencilView* GetDepth();

	private:
		friend class FullscreenTextureFactory;
		friend class GBuffer;
		friend class FullscreenTextureCube;

		ID3D11DeviceContext* myContext = nullptr;

		ComPtr<ID3D11RenderTargetView> myRenderTarget = nullptr;
		ComPtr<ID3D11DepthStencilView> myDepth = nullptr;

		ComPtr<ID3D11Texture2D> myTexture = nullptr;
		ComPtr<ID3D11ShaderResourceView> mySRV = nullptr;
		D3D11_VIEWPORT* myViewport = nullptr;
	};
}