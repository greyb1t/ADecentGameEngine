#include "pch.h"
#include "CubemapTexture.h"
#include "Engine\Engine.h"
#include "Engine\Renderer\GraphicsEngine.h"
#include "Engine\Renderer\Directx11Framework.h"

bool Engine::CubemapTexture::Init(const Vec2ui& aSize)
{
	if (myIsCreated)
	{
		return true;
	}

	myIsCreated = true;

	mySize = aSize;

	myMipLevelCount = static_cast<int>(std::floor(log2(std::min(aSize.x, aSize.y)))) + 1;

	assert(!(aSize.x == 0) && !(aSize.x & (aSize.x - 1)) && "must be power of 2");
	assert(!(aSize.y == 0) && !(aSize.y & (aSize.y - 1)) && "must be power of 2");

	// To support HDR cubemap
	const DXGI_FORMAT format = DXGI_FORMAT_R16G16B16A16_FLOAT;

	HRESULT result = 0;

	D3D11_TEXTURE2D_DESC textureDesc = { };
	{
		textureDesc.Width = aSize.x;
		textureDesc.Height = aSize.y;
		textureDesc.MipLevels = 0; // 0 for full set of mip levels
		textureDesc.Format = format;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags =
			D3D11_BIND_SHADER_RESOURCE |
			D3D11_BIND_RENDER_TARGET;
		textureDesc.CPUAccessFlags = 0;

		textureDesc.ArraySize = 6;
		textureDesc.MiscFlags =
			D3D11_RESOURCE_MISC_TEXTURECUBE |
			D3D11_RESOURCE_MISC_GENERATE_MIPS;
	}

	auto device = GetEngine().GetGraphicsEngine().GetDxFramework().GetDevice();

	result = device->CreateTexture2D(&textureDesc, nullptr, &myTexture);
	assert(SUCCEEDED(result));

	// Create texture cube views to later on me sampled from
	{
		D3D11_RENDER_TARGET_VIEW_DESC cubeRtvDesc = { };
		{
			cubeRtvDesc.Format = format;
			cubeRtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
			cubeRtvDesc.Texture2DArray.FirstArraySlice = 0;
			cubeRtvDesc.Texture2DArray.ArraySize = 6;
		}

		result = device->CreateRenderTargetView(myTexture.Get(), &cubeRtvDesc, &myCubeRTV);

		assert(SUCCEEDED(result));

		D3D11_SHADER_RESOURCE_VIEW_DESC cubeSrvDesc = { };
		{
			cubeSrvDesc.Format = format;
			cubeSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;

			// -1 for all the mip levels
			cubeSrvDesc.TextureCube.MipLevels = -1;
			// ? unsure of this value, doc says -1 for something detail...
			cubeSrvDesc.TextureCube.MostDetailedMip = 0;
		}

		result = device->CreateShaderResourceView(myTexture.Get(), &cubeSrvDesc, &myCubeSRV);
		assert(SUCCEEDED(result));
	}

	// TODO: We can remove this later on, because we do it in 1 drawcall only
	// Create each side of cube views to later on be render to
	// ID3D11ShaderResourceView* sideSRVs[6] = { };

	// Create 6 cube face texture RTV for each mip level
	for (int mipLevel = 0; mipLevel < myMipLevelCount; ++mipLevel)
	{
		const int cubeSideCount = 6;

		MipLevel mip;

		for (int i = 0; i < cubeSideCount; ++i)
		{
			D3D11_RENDER_TARGET_VIEW_DESC sideRtvDesc = { };
			{
				sideRtvDesc.Format = format;
				sideRtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
				sideRtvDesc.Texture2DArray.FirstArraySlice = i;
				sideRtvDesc.Texture2DArray.ArraySize = 1;
				sideRtvDesc.Texture2DArray.MipSlice = mipLevel;
			}

			device->CreateRenderTargetView(myTexture.Get(), &sideRtvDesc, &mip.mySideRTVs[i]);

			assert(SUCCEEDED(result));

			mip.myMipLevel = mipLevel;

			mip.myViewport.TopLeftX = 0.f;
			mip.myViewport.TopLeftY = 0.f;
			mip.myViewport.MinDepth = 0.f;
			mip.myViewport.MaxDepth = 1.f;
			mip.myViewport.Width = static_cast<float>(aSize.x) * std::pow(0.5f, mipLevel);
			mip.myViewport.Height = static_cast<float>(aSize.y) * std::pow(0.5f, mipLevel);

			/*
			D3D11_SHADER_RESOURCE_VIEW_DESC resourceDesc = { };
			{
				resourceDesc.Format = format;
				resourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

				// -1 for all the mip levels
				resourceDesc.Texture2D.MipLevels = -1;
				// ? unsure of this value, doc says -1 for something detail...
				resourceDesc.Texture2D.MostDetailedMip = 0;
			}

			result = device->CreateShaderResourceView(texture, &resourceDesc, &sideSRVs[i]);
			assert(SUCCEEDED(result));
			*/
		}

		myMipLevels.push_back(mip);
	}

	// TODO: Free this when? Why even allocated on heap?
	myViewport = DBG_NEW D3D11_VIEWPORT;
	{
		myViewport->TopLeftX = 0.f;
		myViewport->TopLeftY = 0.f;
		myViewport->Width = static_cast<float>(aSize.x);
		myViewport->Height = static_cast<float>(aSize.y);
		myViewport->MinDepth = 0.f;
		myViewport->MaxDepth = 1.f;
	};

	/*
	std::copy(std::begin(sideSRVs), std::end(sideSRVs), returnDepth.mySideSRVs);
	std::copy(std::begin(sideRTVs), std::end(sideRTVs), returnDepth.mySideRTVs);
	*/

	return true;
}

const Engine::CubemapTexture::MipLevel& Engine::CubemapTexture::GetMipLevel(const int aMipLevel) const
{
	return myMipLevels[aMipLevel];
}

int Engine::CubemapTexture::GetMipLevelCount() const
{
	return myMipLevelCount;
}

ID3D11Texture2D* Engine::CubemapTexture::GetTexture2D()
{
	return myTexture.Get();
}

ID3D11ShaderResourceView* Engine::CubemapTexture::GetCubeSRV()
{
	return myCubeSRV.Get();
}
