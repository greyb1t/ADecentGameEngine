#include "pch.h"
#include "EffectPassState.h"

void Engine::EffectPassState::SetDepthStencil(ID3D11DepthStencilState* aDepthStencil, const UINT aStencilRef)
{
	myDepthStencil = aDepthStencil;
}

void Engine::EffectPassState::SetRasterizerState(ID3D11RasterizerState* aRasterizerState)
{
	myRasterizerState = aRasterizerState;
}

void Engine::EffectPassState::SetBlendState(ID3D11BlendState* aBlendState)
{
	myBlendState = aBlendState;
}

void Engine::EffectPassState::ResetState(ID3D11DeviceContext& aContext)
{
	if (myDepthStencil)
	{
		aContext.OMSetDepthStencilState(myDepthStencil, myStencilRef);
	}

	if (myRasterizerState)
	{
		aContext.RSSetState(myRasterizerState);
	}

	if (myBlendState)
	{
		// TODO: do not hardcode these?
		UINT mask = 0xffffffff;
		const FLOAT blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
		aContext.OMSetBlendState(myBlendState, blendFactor, mask);
	}
}
