#pragma once

#include "Engine/Renderer/Texture/FullscreenTexture.h"
#include "Engine/Renderer/Texture/FullscreenTextureCube.h"
#include "Engine/Renderer/GBuffer.h"

namespace Engine
{
	class WindowHandler;

	class FullscreenTextureFactory
	{
	public:
		bool Init(
			ID3D11Device* aDevice, 
			ID3D11DeviceContext* aContext);

		FullscreenTexture CreateTexture(
			const C::Vector2<unsigned int>& aSize, 
			DXGI_FORMAT aFormat, 
			D3D11_SUBRESOURCE_DATA* aData);
		FullscreenTexture CreateTexture(const C::Vector2<unsigned int>& aSize, DXGI_FORMAT aFormat);
		FullscreenTexture CreateTextureMipMaps(const C::Vector2<unsigned int>& aSize, DXGI_FORMAT aFormat);
		FullscreenTexture CreateTexture(ID3D11Texture2D* aTextureTemplate);
		FullscreenTexture CreateDepth(const C::Vector2<unsigned int>& aSize, DXGI_FORMAT aFormat);
		FullscreenTexture CreateShadowMap(const C::Vector2<unsigned int>& aSize);
		FullscreenTextureCube CreateShadowMapCube(const C::Vector2<unsigned int>& aSize);
		FullscreenTextureCube CreateCubemapTexture(const C::Vector2<unsigned int>& aSize, const bool aCanGenerateMips);
		GBuffer CreateGBuffer(const C::Vector2<unsigned int>& aSize);

	private:
		ID3D11Device* myDevice = nullptr;
		ID3D11DeviceContext* myContext = nullptr;
	};
}