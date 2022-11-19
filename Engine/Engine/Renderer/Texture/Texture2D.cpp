#include "pch.h"
#include "Texture2D.h"

Engine::Texture2D::Texture2D()
{
}

Engine::Texture2D::Texture2D(
	ComPtr<ID3D11Texture2D> aTexture2D, 
	ComPtr<ID3D11ShaderResourceView> aSRV, 
	const Vec2ui& aTextureSize)
	: myTexture2D(aTexture2D),
	mySRV(aSRV),
	mySize(aTextureSize)
{
}

Engine::Texture2D::~Texture2D()
{
	int test = 0;
}

ID3D11ShaderResourceView* Engine::Texture2D::GetSRV()
{
	return mySRV.Get();
}

ID3D11ShaderResourceView** Engine::Texture2D::GetSRVPtr()
{
	return mySRV.GetAddressOf();
}

ID3D11Texture2D* Engine::Texture2D::GetTexture2D()
{
	return myTexture2D.Get();
}

ID3D11Texture2D** Engine::Texture2D::GetTexture2DPtr()
{
	return myTexture2D.GetAddressOf();
}

const Vec2ui& Engine::Texture2D::GetImageSize() const
{
	return mySize;
}
