#include "pch.h"
#include "FullscreenTextureCube.h"
#include "Engine/Renderer/Directx11Framework.h"
#include "FullscreenTexture.h"

namespace Engine
{
	FullscreenTextureCube::FullscreenTextureCube()
	{

	}

	FullscreenTextureCube::FullscreenTextureCube(ID3D11DeviceContext* aContext)
		: myContext(aContext)
	{
	}

	void FullscreenTextureCube::ClearTextureWholeCube(C::Vector4f aClearColor)
	{
		myContext->ClearRenderTargetView(myCubeRTV.Get(), &aClearColor.x);
	}

	void FullscreenTextureCube::ClearDepthWholeCube(float aClearDepthValue /*= 1.0f*/, unsigned int aClearStencilValue /*= 0*/)
	{
		myContext->ClearDepthStencilView(
			myCubeDSV.Get(),
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
			aClearDepthValue,
			static_cast<UINT8>(aClearStencilValue));
	}

	void FullscreenTextureCube::SetAsActiveTarget(FullscreenTextureCube* aDepth)
	{
		assert(false);
	}

	void FullscreenTextureCube::SetAsActiveTargetSide(const int aIndex, FullscreenTexture* aDepth)
	{
		if (aDepth)
		{
			myContext->OMSetRenderTargets(1, mySideRTVs[aIndex].GetAddressOf(), aDepth->myDepth.Get());
			myContext->RSSetViewports(1, myViewport);
		}
		else
		{
			myContext->OMSetRenderTargets(1, mySideRTVs[aIndex].GetAddressOf(), nullptr);
			myContext->RSSetViewports(1, myViewport);
		}
	}

	void FullscreenTextureCube::SetAsActiveDepthSide(const int aIndex)
	{
		myContext->OMSetRenderTargets(0, nullptr, mySideDSVs[aIndex].Get());
		myContext->RSSetViewports(1, myViewport);
	}

	void FullscreenTextureCube::SetAsActiveDepth()
	{
		myContext->OMSetRenderTargets(0, nullptr, myCubeDSV.Get());
		myContext->RSSetViewports(1, myViewport);
	}

	void FullscreenTextureCube::SetCubeAsResourceOnSlot(unsigned int aSlot)
	{
		myContext->PSSetShaderResources(aSlot, 1, myCubeSRV.GetAddressOf());
	}

	ID3D11Texture2D* FullscreenTextureCube::GetTexture2D()
	{
		return myTexture.Get();
	}

	ID3D11ShaderResourceView* FullscreenTextureCube::GetCubeSRV()
	{
		return myCubeSRV.Get();
	}

	ID3D11ShaderResourceView** FullscreenTextureCube::GetCubeSRVPtr()
	{
		return myCubeSRV.GetAddressOf();
	}

	//void FullscreenTextureCube::SetAsResourceOnSlot(unsigned int aSlot)
	//{
	//	assert(false);
	//	//myContext->PSSetShaderResources(aSlot, 1, &mySRV);
	//}
}