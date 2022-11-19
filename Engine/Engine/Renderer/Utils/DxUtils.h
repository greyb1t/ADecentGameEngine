#pragma once

namespace Engine
{
	void ThrowIfFailed(const HRESULT aResult);

	namespace DxUtils
	{
		template <typename T>
		void SafeRelease(T** aValue);

		void UnbindShaderResourceView(ID3D11DeviceContext& aContext, const int aSlot);
		void UnbindRenderTargetView(ID3D11DeviceContext& aContext);
	}

	template <typename T>
	void DxUtils::SafeRelease(T** aValue)
	{
		if (*aValue != nullptr)
		{
			(*aValue)->Release();
			*aValue = nullptr;
		}
	}
}