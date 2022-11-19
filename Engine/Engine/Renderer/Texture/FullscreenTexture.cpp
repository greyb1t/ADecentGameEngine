#include "pch.h"
#include "FullscreenTexture.h"
#include "Engine\Renderer\Directx11Framework.h"
#include "Engine\Renderer\Utils\DxUtils.h"

namespace Engine
{
	FullscreenTexture::FullscreenTexture(ID3D11DeviceContext& aContext)
		: myContext(&aContext)
	{
	}

	FullscreenTexture::FullscreenTexture()
	{
	}

	FullscreenTexture::~FullscreenTexture()
	{
		// DxUtils::SafeRelease(&myRenderTarget);
		// DxUtils::SafeRelease(&myDepth);

		// Release();
	}

	void FullscreenTexture::ClearTexture(C::Vector4f aClearColor)
	{
		myContext->ClearRenderTargetView(myRenderTarget.Get(), &aClearColor.x);
	}

	void FullscreenTexture::ClearDepth(float aClearDepthValue, unsigned int aClearStencilValue)
	{
		myContext->ClearDepthStencilView(myDepth.Get(),
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
			aClearDepthValue,
			static_cast<UINT8>(aClearStencilValue));
	}

	void FullscreenTexture::SetAsActiveTarget(FullscreenTexture* aDepth)
	{
		if (aDepth != nullptr)
		{
			myContext->OMSetRenderTargets(1, myRenderTarget.GetAddressOf(), aDepth->myDepth.Get());
		}
		else
		{
			myContext->OMSetRenderTargets(1, myRenderTarget.GetAddressOf(), nullptr);
		}

		myContext->RSSetViewports(1, myViewport);
	}

	void FullscreenTexture::SetAsResourceOnSlot(unsigned int aSlot)
	{
		myContext->PSSetShaderResources(aSlot, 1, mySRV.GetAddressOf());
	}

	void FullscreenTexture::SetAsActiveDepth()
	{
		myContext->OMSetRenderTargets(0, nullptr, myDepth.Get());
		myContext->RSSetViewports(1, myViewport);
	}

	ID3D11Texture2D* FullscreenTexture::GetTextureTest()
	{
		return myTexture.Get();
	}

	ID3D11RenderTargetView* FullscreenTexture::GetRenderTargetTest()
	{
		return myRenderTarget.Get();
	}

	ID3D11RenderTargetView** FullscreenTexture::GetRenderTargetTestPtr()
	{
		return myRenderTarget.GetAddressOf();
	}

	ID3D11ShaderResourceView* FullscreenTexture::GetSRV()
	{
		return mySRV.Get();
	}

	ID3D11ShaderResourceView** FullscreenTexture::GetSRVAddressOf()
	{
		return mySRV.GetAddressOf();
	}

	ID3D11DepthStencilView* FullscreenTexture::GetDepth()
	{
		return myDepth.Get();
	}
}