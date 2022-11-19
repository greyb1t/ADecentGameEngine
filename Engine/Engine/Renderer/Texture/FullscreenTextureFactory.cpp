#include "pch.h"
#include "FullscreenTextureFactory.h"
#include "../WindowHandler.h"

namespace Engine
{

	bool FullscreenTextureFactory::Init(
		ID3D11Device* aDevice,
		ID3D11DeviceContext* aContext)
	{
		myDevice = aDevice;
		myContext = aContext;

		return true;
	}

	FullscreenTexture FullscreenTextureFactory::CreateTexture(const C::Vector2<unsigned int>& aSize, DXGI_FORMAT aFormat)
	{
		return CreateTexture(aSize, aFormat, nullptr);
	}

	FullscreenTexture FullscreenTextureFactory::CreateTexture(ID3D11Texture2D* aTextureTemplate)
	{
		ComPtr<ID3D11RenderTargetView> renderTargetView = nullptr;
		HRESULT result = myDevice->CreateRenderTargetView(
			aTextureTemplate, nullptr, &renderTargetView);

		if (FAILED(result))
		{
			LOG_ERROR(LogType::Renderer) << "CreateRenderTargetView failed " << result;
			assert(false && "unhandled");
			// yikes
			return FullscreenTexture(*myContext);
		}

		D3D11_VIEWPORT* viewport = nullptr;

		if (aTextureTemplate != nullptr)
		{
			D3D11_TEXTURE2D_DESC desc = { };
			aTextureTemplate->GetDesc(&desc);

			// TODO: Free this when?
			viewport = new D3D11_VIEWPORT;
			{
				viewport->TopLeftX = 0;
				viewport->TopLeftY = 0;
				viewport->Width = static_cast<float>(desc.Width);
				viewport->Height = static_cast<float>(desc.Height);
				viewport->MinDepth = 0;
				viewport->MaxDepth = 1;
			};
		}

		FullscreenTexture textureResult(*myContext);
		textureResult.myTexture = aTextureTemplate;
		textureResult.myRenderTarget = renderTargetView;

		textureResult.myViewport = viewport;
		return textureResult;
	}

	Engine::FullscreenTexture FullscreenTextureFactory::CreateTexture(const C::Vector2<unsigned int>& aSize, 
		DXGI_FORMAT aFormat, 
		D3D11_SUBRESOURCE_DATA* aData)
	{
		D3D11_TEXTURE2D_DESC desc = { };
		{
			desc.Width = aSize.x;
			desc.Height = aSize.y;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = aFormat;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = 0;
		}

		ComPtr<ID3D11Texture2D> texture = nullptr;
		HRESULT result = myDevice->CreateTexture2D(&desc, aData, &texture);

		if (FAILED(result))
		{
			LOG_ERROR(LogType::Renderer) << "CreateTexture2D failed " << result;
			assert(false && "unhandled");
			// yikes
			return FullscreenTexture(*myContext);
		}

		FullscreenTexture textureResult = CreateTexture(texture.Get());

		ComPtr<ID3D11ShaderResourceView> srv = nullptr;
		result = myDevice->CreateShaderResourceView(texture.Get(), nullptr, &srv);

		if (FAILED(result))
		{
			LOG_ERROR(LogType::Renderer) << "CreateShaderResourceView failed " << result;
			assert(false && "unhandled");
			// yikes
			return FullscreenTexture(*myContext);
		}

		textureResult.mySRV = srv;

		return textureResult;
	}

	Engine::FullscreenTexture FullscreenTextureFactory::CreateTextureMipMaps(const C::Vector2<unsigned int>& aSize, DXGI_FORMAT aFormat)
	{
		D3D11_TEXTURE2D_DESC desc = { };
		{
			desc.Width = aSize.x;
			desc.Height = aSize.y;
			desc.MipLevels = 0;
			desc.ArraySize = 1;
			desc.Format = aFormat;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
		}

		ComPtr<ID3D11Texture2D> texture = nullptr;
		HRESULT result = myDevice->CreateTexture2D(&desc, nullptr, &texture);

		if (FAILED(result))
		{
			LOG_ERROR(LogType::Renderer) << "CreateTexture2D failed " << result;
			assert(false && "unhandled");
			// yikes
			return FullscreenTexture(*myContext);
		}

		FullscreenTexture textureResult = CreateTexture(texture.Get());

		ComPtr<ID3D11ShaderResourceView> srv = nullptr;
		result = myDevice->CreateShaderResourceView(texture.Get(), nullptr, &srv);

		if (FAILED(result))
		{
			LOG_ERROR(LogType::Renderer) << "CreateShaderResourceView failed " << result;
			assert(false && "unhandled");
			// yikes
			return FullscreenTexture(*myContext);
		}

		textureResult.mySRV = srv;

		return textureResult;
	}

	FullscreenTexture FullscreenTextureFactory::CreateDepth(const C::Vector2<unsigned int>& aSize, DXGI_FORMAT aFormat)
	{
		D3D11_TEXTURE2D_DESC desc = { };
		{
			desc.Width = aSize.x;
			desc.Height = aSize.y;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = aFormat;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE /* when making fog, we need the depth texture */;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = 0;
		}

		ComPtr<ID3D11Texture2D> texture = nullptr;
		HRESULT result = myDevice->CreateTexture2D(&desc, nullptr, &texture);

		if (FAILED(result))
		{
			LOG_ERROR(LogType::Renderer) << "CreateTexture2D failed " << result;
			assert(false && "unhandled");
			// yikes
			return FullscreenTexture(*myContext);
		}

		D3D11_DEPTH_STENCIL_VIEW_DESC depthDesc = { };
		{
			depthDesc.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
			depthDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		}

		ComPtr<ID3D11DepthStencilView> depthStencilView = nullptr;
		result = myDevice->CreateDepthStencilView(texture.Get(), &depthDesc, &depthStencilView);

		if (FAILED(result))
		{
			LOG_ERROR(LogType::Renderer) << "CreateDepthStencilView failed " << result;
			assert(false && "unhandled");
			// yikes
			return FullscreenTexture(*myContext);
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC resourceDesc = { };
		{
			resourceDesc.Format = /*DXGI_FORMAT_R24_UNORM_X8_TYPELESS*/DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
			resourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			resourceDesc.Texture2D.MipLevels = desc.MipLevels;
		}

		ComPtr<ID3D11ShaderResourceView> shaderResource = nullptr;
		result = myDevice->CreateShaderResourceView(texture.Get(), &resourceDesc, &shaderResource);
		assert(SUCCEEDED(result));

		// TODO: Free this when? Why even allocated on heap?
		D3D11_VIEWPORT* viewport = DBG_NEW D3D11_VIEWPORT;
		{
			viewport->TopLeftX = 0;
			viewport->TopLeftY = 0;
			viewport->Width = static_cast<float>(desc.Width);
			viewport->Height = static_cast<float>(desc.Height);
			viewport->MinDepth = 0;
			viewport->MaxDepth = 1;
		};

		FullscreenTexture textureResult(*myContext);
		textureResult.myTexture = texture;
		textureResult.myDepth = depthStencilView;
		textureResult.mySRV = shaderResource;
		textureResult.myViewport = viewport;
		return textureResult;
	}

	FullscreenTexture FullscreenTextureFactory::CreateShadowMap(const C::Vector2<unsigned int>& aSize)
	{
		HRESULT result = 0;

		D3D11_TEXTURE2D_DESC textureDesc = { };
		{
			textureDesc.Width = aSize.x;
			textureDesc.Height = aSize.y;
			textureDesc.MipLevels = 1;
			textureDesc.ArraySize = 1;
			textureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			textureDesc.SampleDesc.Count = 1;
			textureDesc.SampleDesc.Quality = 0;
			textureDesc.Usage = D3D11_USAGE_DEFAULT;
			textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
			textureDesc.CPUAccessFlags = 0;
			textureDesc.MiscFlags = 0;
		}

		ComPtr<ID3D11Texture2D> texture = nullptr;
		result = myDevice->CreateTexture2D(&textureDesc, nullptr, &texture);
		assert(SUCCEEDED(result));

		D3D11_DEPTH_STENCIL_VIEW_DESC depthDesc = { };
		{
			depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
			depthDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		}

		ComPtr<ID3D11DepthStencilView> depth = nullptr;
		result = myDevice->CreateDepthStencilView(texture.Get(), &depthDesc, &depth);
		assert(SUCCEEDED(result));

		D3D11_SHADER_RESOURCE_VIEW_DESC resourceDesc = { };
		{
			resourceDesc.Format = DXGI_FORMAT_R32_FLOAT;
			resourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			resourceDesc.Texture2D.MipLevels = textureDesc.MipLevels;
		}

		ComPtr<ID3D11ShaderResourceView> shaderResource = nullptr;
		result = myDevice->CreateShaderResourceView(texture.Get(), &resourceDesc, &shaderResource);
		assert(SUCCEEDED(result));

		// TODO: Free this when? Why even allocated on heap?
		D3D11_VIEWPORT* viewport = DBG_NEW D3D11_VIEWPORT;
		{
			viewport->TopLeftX = 0.f;
			viewport->TopLeftY = 0.f;
			viewport->Width = static_cast<float>(aSize.x);
			viewport->Height = static_cast<float>(aSize.y);
			viewport->MinDepth = 0.f;
			viewport->MaxDepth = 1.f;
		};

		FullscreenTexture returnDepth(*myContext);
		returnDepth.myTexture = texture;
		returnDepth.myDepth = depth;
		returnDepth.mySRV = shaderResource;
		returnDepth.myViewport = viewport;

		return returnDepth;
	}

	FullscreenTextureCube FullscreenTextureFactory::CreateShadowMapCube(const C::Vector2<unsigned int>& aSize)
	{
		assert(!(aSize.x == 0) && !(aSize.x & (aSize.x - 1)) && "must be power of 2");
		assert(!(aSize.y == 0) && !(aSize.y & (aSize.y - 1)) && "must be power of 2");

		HRESULT result = 0;

		D3D11_TEXTURE2D_DESC textureDesc = { };
		{
			// OMG: When doing a texture cube, Width and Height must be power of 2?
			textureDesc.Width = aSize.x;
			textureDesc.Height = aSize.y;
			textureDesc.MipLevels = 1;
			textureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			textureDesc.SampleDesc.Count = 1;
			textureDesc.SampleDesc.Quality = 0;
			textureDesc.Usage = D3D11_USAGE_DEFAULT;
			textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
			textureDesc.CPUAccessFlags = 0;

			textureDesc.ArraySize = 6;
			textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
		}

		ComPtr<ID3D11Texture2D> texture = nullptr;
		result = myDevice->CreateTexture2D(&textureDesc, nullptr, &texture);
		assert(SUCCEEDED(result));

		// Create texture cube views to later on me sampled from
		ComPtr<ID3D11ShaderResourceView> cubeSRV = nullptr;
		ComPtr<ID3D11DepthStencilView> cubeDSV = nullptr;
		{
			D3D11_DEPTH_STENCIL_VIEW_DESC depthDesc = { };
			{
				depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
				depthDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
				depthDesc.Texture2DArray.FirstArraySlice = 0;
				depthDesc.Texture2DArray.ArraySize = 6;
			}

			result = myDevice->CreateDepthStencilView(texture.Get(), &depthDesc, &cubeDSV);
			assert(SUCCEEDED(result));

			D3D11_SHADER_RESOURCE_VIEW_DESC resourceDesc = { };
			{
				resourceDesc.Format = DXGI_FORMAT_R32_FLOAT;
				resourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;

				resourceDesc.TextureCube.MipLevels = textureDesc.MipLevels;
				// ? unsure of this value, doc says -1 for something detail...
				resourceDesc.TextureCube.MostDetailedMip = 0;
			}

			result = myDevice->CreateShaderResourceView(texture.Get(), &resourceDesc, &cubeSRV);
			assert(SUCCEEDED(result));
		}

		// TODO: We can remove this later on, because we do it in 1 drawcall only
		// Create each side of cube views to later on be render to
		ComPtr<ID3D11ShaderResourceView> sideSRVs[6] = { };
		ComPtr<ID3D11DepthStencilView> sideDSVs[6] = { };
		{
			const int cubeSideCount = 6;

			for (int i = 0; i < cubeSideCount; ++i)
			{
				D3D11_DEPTH_STENCIL_VIEW_DESC depthDesc = { };
				{
					depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
					depthDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
					depthDesc.Texture2DArray.FirstArraySlice = i;
					depthDesc.Texture2DArray.ArraySize = 1;
				}

				result = myDevice->CreateDepthStencilView(texture.Get(), &depthDesc, &sideDSVs[i]);
				assert(SUCCEEDED(result));

				D3D11_SHADER_RESOURCE_VIEW_DESC resourceDesc = { };
				{
					resourceDesc.Format = DXGI_FORMAT_R32_FLOAT;
					resourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

					resourceDesc.Texture2D.MipLevels = textureDesc.MipLevels;
					// ? unsure of this value, doc says -1 for something detail...
					resourceDesc.Texture2D.MostDetailedMip = 0;
				}

				result = myDevice->CreateShaderResourceView(texture.Get(), &resourceDesc, &sideSRVs[i]);
				assert(SUCCEEDED(result));
			}
		}

		// TODO: Free this when? Why even allocated on heap?
		D3D11_VIEWPORT* viewport = DBG_NEW D3D11_VIEWPORT;
		{
			viewport->TopLeftX = 0.f;
			viewport->TopLeftY = 0.f;
			viewport->Width = static_cast<float>(aSize.x);
			viewport->Height = static_cast<float>(aSize.y);
			viewport->MinDepth = 0.f;
			viewport->MaxDepth = 1.f;
		};

		FullscreenTextureCube returnDepth(myContext);

		returnDepth.myTexture = texture;
		returnDepth.myCubeDSV = cubeDSV;
		returnDepth.myCubeSRV = cubeSRV;
		returnDepth.myViewport = viewport;

		std::copy(std::begin(sideSRVs), std::end(sideSRVs), returnDepth.mySideSRVs);
		std::copy(std::begin(sideDSVs), std::end(sideDSVs), returnDepth.mySideDSVs);

		return returnDepth;
	}

	Engine::FullscreenTextureCube FullscreenTextureFactory::CreateCubemapTexture(const C::Vector2<unsigned int>& aSize, const bool aCanGenerateMips)
	{
		assert(!(aSize.x == 0) && !(aSize.x & (aSize.x - 1)) && "must be power of 2");
		assert(!(aSize.y == 0) && !(aSize.y & (aSize.y - 1)) && "must be power of 2");

		const DXGI_FORMAT format = DXGI_FORMAT_R16G16B16A16_FLOAT;

		HRESULT result = 0;

		D3D11_TEXTURE2D_DESC textureDesc = { };
		{
			textureDesc.Width = aSize.x;
			textureDesc.Height = aSize.y;
			if (aCanGenerateMips)
			{
				// full set of sub textures (mip maps)
				textureDesc.MipLevels = 0;
			}
			else
			{
				textureDesc.MipLevels = 1;
			}
			textureDesc.Format = format; // To support HDR cubemap?
			textureDesc.SampleDesc.Count = 1;
			textureDesc.SampleDesc.Quality = 0;
			textureDesc.Usage = D3D11_USAGE_DEFAULT;
			textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
			textureDesc.CPUAccessFlags = 0;

			textureDesc.ArraySize = 6;
			textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

			if (aCanGenerateMips)
			{
				textureDesc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
			}
		}

		ComPtr<ID3D11Texture2D> texture = nullptr;
		result = myDevice->CreateTexture2D(&textureDesc, nullptr, &texture);
		assert(SUCCEEDED(result));

		// Create texture cube views to later on me sampled from
		ComPtr<ID3D11ShaderResourceView> cubeSRV = nullptr;
		ComPtr<ID3D11RenderTargetView> cubeRTV = nullptr;
		{
			D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = { };
			{
				rtvDesc.Format = format;
				rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
				rtvDesc.Texture2DArray.FirstArraySlice = 0;
				rtvDesc.Texture2DArray.ArraySize = 6;
			}

			result = myDevice->CreateRenderTargetView(texture.Get(), &rtvDesc, &cubeRTV);

			assert(SUCCEEDED(result));

			D3D11_SHADER_RESOURCE_VIEW_DESC resourceDesc = { };
			{
				resourceDesc.Format = format;
				resourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;

				// -1 for all the mip levels
				resourceDesc.TextureCube.MipLevels = -1;
				// ? unsure of this value, doc says -1 for something detail...
				resourceDesc.TextureCube.MostDetailedMip = 0;
			}

			result = myDevice->CreateShaderResourceView(texture.Get(), &resourceDesc, &cubeSRV);
			assert(SUCCEEDED(result));
		}

		// TODO: We can remove this later on, because we do it in 1 drawcall only
		// Create each side of cube views to later on be render to
		ComPtr<ID3D11ShaderResourceView> sideSRVs[6] = { };
		ComPtr<ID3D11RenderTargetView> sideRTVs[6] = { };
		{
			const int cubeSideCount = 6;

			for (int i = 0; i < cubeSideCount; ++i)
			{
				D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = { };
				{
					rtvDesc.Format = format;
					rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
					rtvDesc.Texture2DArray.FirstArraySlice = i;
					rtvDesc.Texture2DArray.ArraySize = 1;
				}

				myDevice->CreateRenderTargetView(texture.Get(), &rtvDesc, &sideRTVs[i]);

				assert(SUCCEEDED(result));

				D3D11_SHADER_RESOURCE_VIEW_DESC resourceDesc = { };
				{
					resourceDesc.Format = format;
					resourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

					// -1 for all the mip levels
					resourceDesc.Texture2D.MipLevels = -1;
					// ? unsure of this value, doc says -1 for something detail...
					resourceDesc.Texture2D.MostDetailedMip = 0;
				}

				result = myDevice->CreateShaderResourceView(texture.Get(), &resourceDesc, &sideSRVs[i]);
				assert(SUCCEEDED(result));
			}
		}

		// TODO: Free this when? Why even allocated on heap?
		D3D11_VIEWPORT* viewport = DBG_NEW D3D11_VIEWPORT;
		{
			viewport->TopLeftX = 0.f;
			viewport->TopLeftY = 0.f;
			viewport->Width = static_cast<float>(aSize.x);
			viewport->Height = static_cast<float>(aSize.y);
			viewport->MinDepth = 0.f;
			viewport->MaxDepth = 1.f;
		};

		FullscreenTextureCube returnDepth(myContext);

		returnDepth.myTexture = texture;
		returnDepth.myCubeRTV = cubeRTV;
		returnDepth.myCubeSRV = cubeSRV;
		returnDepth.myViewport = viewport;

		std::copy(std::begin(sideSRVs), std::end(sideSRVs), returnDepth.mySideSRVs);
		std::copy(std::begin(sideRTVs), std::end(sideRTVs), returnDepth.mySideRTVs);

		return returnDepth;
	}

	GBuffer FullscreenTextureFactory::CreateGBuffer(const C::Vector2<unsigned int>& aSize)
	{
		std::array<DXGI_FORMAT, GBuffer::Count> textureFormats =
		{
			DXGI_FORMAT_R32G32B32A32_FLOAT, // Position
			DXGI_FORMAT_R16G16B16A16_FLOAT, // Albedo
			DXGI_FORMAT_R16G16B16A16_FLOAT,	// Normal
			DXGI_FORMAT_R16G16B16A16_FLOAT, // VertexNormal

			// NOTE(Filip): changed material from R8G8B8A8_UNORM to 16_float to support emissive intensity in the gbuffer
			DXGI_FORMAT_R16G16B16A16_FLOAT,		// Material

			/*DXGI_FORMAT_R8_UNORM*/DXGI_FORMAT_R16G16B16A16_FLOAT,	// AmbientOcclusion.r & Emissive.gba
			DXGI_FORMAT_R16_UNORM			// Depth
		};

		GBuffer gbufferResult;

		gbufferResult.Init(myContext);

		D3D11_TEXTURE2D_DESC desc = { };
		{
			desc.Width = aSize.x;
			desc.Height = aSize.y;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = 0;
		}

		for (unsigned int i = 0; i < GBuffer::Count; i++)
		{
			desc.Format = textureFormats[i];
			HRESULT result = myDevice->CreateTexture2D(&desc, nullptr, &gbufferResult.myTextures[i]);
			assert(SUCCEEDED(result));

			result = myDevice->CreateRenderTargetView(gbufferResult.myTextures[i].Get(), nullptr, &gbufferResult.myRTVs[i]);
			assert(SUCCEEDED(result));

			result = myDevice->CreateShaderResourceView(gbufferResult.myTextures[i].Get(), nullptr, &gbufferResult.mySRVs[i]);
			assert(SUCCEEDED(result));
		}

		gbufferResult.myViewport = DBG_NEW D3D11_VIEWPORT;
		{
			gbufferResult.myViewport->TopLeftX = 0;
			gbufferResult.myViewport->TopLeftY = 0;
			gbufferResult.myViewport->Width = static_cast<float>(desc.Width);
			gbufferResult.myViewport->Height = static_cast<float>(desc.Height);
			gbufferResult.myViewport->MinDepth = 0;
			gbufferResult.myViewport->MaxDepth = 1;
		};

		return gbufferResult;
	}

}