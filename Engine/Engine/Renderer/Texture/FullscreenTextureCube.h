#pragma once

namespace Engine
{
	class Directx11Framework;
	class FullscreenTexture;

	class FullscreenTextureCube
	{
	public:
		FullscreenTextureCube();
		FullscreenTextureCube(ID3D11DeviceContext* aContext);
		~FullscreenTextureCube() = default;

		void ClearTextureWholeCube(C::Vector4f aClearColor = { 0.f, 0.f, 0.f, 0.f });
		void ClearDepthWholeCube(float aClearDepthValue = 1.0f, unsigned int aClearStencilValue = 0);
		void SetAsActiveTarget(FullscreenTextureCube* aDepth = nullptr);
		void SetAsActiveTargetSide(const int aIndex, FullscreenTexture* aDepth);
		void SetAsActiveDepthSide(const int aIndex);
		void SetAsActiveDepth();
		//void SetAsResourceOnSlot(unsigned int aSlot);
		void SetCubeAsResourceOnSlot(unsigned int aSlot);

		ID3D11Texture2D* GetTexture2D();

		ID3D11ShaderResourceView* GetCubeSRV();
		ID3D11ShaderResourceView** GetCubeSRVPtr();

	private:
		friend class FullscreenTextureFactory;

		ID3D11DeviceContext* myContext = nullptr;

		ComPtr<ID3D11RenderTargetView> myCubeRTV = nullptr;
		ComPtr<ID3D11DepthStencilView> myCubeDSV;


		ComPtr<ID3D11RenderTargetView> mySideRTVs[6] = { };
		ComPtr<ID3D11DepthStencilView> mySideDSVs[6];


		ComPtr<ID3D11Texture2D> myTexture;

		ComPtr<ID3D11ShaderResourceView> myCubeSRV = nullptr;
		ComPtr<ID3D11ShaderResourceView> mySideSRVs[6] = { };

		D3D11_VIEWPORT* myViewport = nullptr;
	};
}