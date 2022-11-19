#pragma once

#include "TextureType.h"

namespace Engine
{
	class Texture2D
	{
	public:
		Texture2D();

		Texture2D(
			ComPtr<ID3D11Texture2D> aTexture2D, 
			ComPtr<ID3D11ShaderResourceView> aSRV, 
			const Vec2ui& aTextureSize);

		~Texture2D();

		ID3D11ShaderResourceView* GetSRV();
		ID3D11ShaderResourceView** GetSRVPtr();

		ID3D11Texture2D* GetTexture2D();
		ID3D11Texture2D** GetTexture2DPtr();

		const Vec2ui& GetImageSize() const;

	private:
		friend class TextureLoader;

		Vec2ui mySize;

		ComPtr<ID3D11Texture2D> myTexture2D = nullptr;
		ComPtr<ID3D11ShaderResourceView> mySRV = nullptr;

		TextureType myType = TextureType::Unknown;
	};
}