#pragma once

namespace Engine
{
	class CubemapTexture
	{
	public:
		struct MipLevel
		{
			int myMipLevel = 0;
			std::array<ComPtr<ID3D11RenderTargetView>, 6> mySideRTVs;
			D3D11_VIEWPORT myViewport = { };
		};

		bool Init(const Vec2ui& aSize);

		const MipLevel& GetMipLevel(const int aMipLevel) const;
		int GetMipLevelCount() const;

		ID3D11Texture2D* GetTexture2D();
		ID3D11ShaderResourceView* GetCubeSRV();

	private:
		bool myIsCreated = false;

		Vec2ui mySize;
		int myMipLevelCount = 0;

		ComPtr<ID3D11Texture2D> myTexture;

		ComPtr<ID3D11RenderTargetView> myCubeRTV;
		ComPtr<ID3D11ShaderResourceView> myCubeSRV;

		D3D11_VIEWPORT* myViewport = nullptr;

		// index 0: all 6 cube texture sides for mip level 0
		// index 1: all 6 cube texture sides for mip level 1
		// etc..
		std::vector<MipLevel> myMipLevels;
	};
}