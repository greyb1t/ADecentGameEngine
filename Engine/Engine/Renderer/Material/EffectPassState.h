#pragma once

namespace Engine
{
	// Returned from a effect pass bind call to be able to reset the state
	class EffectPassState
	{
	public:
		void SetDepthStencil(ID3D11DepthStencilState* aDepthStencil, const UINT aStencilRef);
		void SetRasterizerState(ID3D11RasterizerState* aRasterizerState);
		void SetBlendState(ID3D11BlendState* aBlendState);

		void ResetState(ID3D11DeviceContext& aContext);

	private:
		ID3D11DepthStencilState* myDepthStencil = nullptr;
		UINT myStencilRef = 0;

		ID3D11RasterizerState* myRasterizerState = nullptr;

		ID3D11BlendState* myBlendState = nullptr;
	};
}