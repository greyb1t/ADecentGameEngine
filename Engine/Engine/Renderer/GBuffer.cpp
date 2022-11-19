#include "pch.h"
#include "GBuffer.h"
#include "Engine/Renderer/Texture/FullscreenTexture.h"

namespace Engine
{
	GBuffer::GBuffer()
	{
	}

	bool GBuffer::Init(ID3D11DeviceContext* aContext)
	{
		myContext = aContext;

		return true;
	}

	void GBuffer::ClearTextures()
	{
		FLOAT blackColor[4] = { 0.f, 0.f, 0.f, 0.f };
		for (int i = 0; i < GBuffer::Count; i++)
		{
			myContext->ClearRenderTargetView(myRTVs[i].Get(), blackColor);
		}
	}

	void GBuffer::SetAsActiveTarget(FullscreenTexture* aDepth)
	{
		if (aDepth != nullptr)
		{
			myContext->OMSetRenderTargets(GBufferTexture::Count, myRTVs[0].GetAddressOf(), aDepth->myDepth.Get());
		}
		else
		{
			myContext->OMSetRenderTargets(GBufferTexture::Count, myRTVs[0].GetAddressOf(), nullptr);
		}

		myContext->RSSetViewports(1, myViewport);
	}

	void GBuffer::SetAsResourceOnSlot(GBufferTexture aTexture, unsigned int aSlot) const
	{
		myContext->PSSetShaderResources(aSlot, 1, mySRVs[aTexture].GetAddressOf());
	}

	void GBuffer::SetAllAsResources()
	{
		myContext->PSSetShaderResources(1, GBufferTexture::Count, mySRVs[0].GetAddressOf());
	}

	void GBuffer::UnbindAllResources()
	{
		ID3D11ShaderResourceView* nullViews[GBufferTexture::Count] = { nullptr };
		myContext->PSSetShaderResources(1, GBufferTexture::Count, nullViews);
	}

	ID3D11RenderTargetView* GBuffer::GetRenderTarget(const GBufferTexture aTexture)
	{
		return myRTVs[static_cast<int>(aTexture)].Get();
	}

}