#pragma once
#include "TextureType.h"

namespace Engine
{
	class Texture2D;

	class TextureLoader
	{
	public:
		TextureLoader();
		~TextureLoader();

		bool Init(ID3D11Device* aDevice, ID3D11DeviceContext& aContext);

		Shared<Texture2D> LoadTexture(const std::string& aPath);
		Shared<Texture2D> LoadHDR(const std::string& aPath);

	private:
		ID3D11Device* myDevice = nullptr;
		ID3D11DeviceContext* myContext = nullptr;
	};
}