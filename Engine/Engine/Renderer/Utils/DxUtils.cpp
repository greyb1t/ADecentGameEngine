#include "pch.h"
#include "DxUtils.h"
#include <system_error>

void Engine::DxUtils::UnbindShaderResourceView(ID3D11DeviceContext& aContext, const int aSlot)
{
	ID3D11ShaderResourceView* nullView[1] = { nullptr };
	aContext.PSSetShaderResources(aSlot, 1, nullView);
}

void Engine::DxUtils::UnbindRenderTargetView(ID3D11DeviceContext& aContext)
{
	ID3D11RenderTargetView* nulledRTV = { nullptr };
	aContext.OMSetRenderTargets(1, &nulledRTV, nullptr);
}

void Engine::ThrowIfFailed(const HRESULT aResult)
{
	if (FAILED(aResult))
	{
		throw std::system_error(aResult, std::generic_category());
	}
}
