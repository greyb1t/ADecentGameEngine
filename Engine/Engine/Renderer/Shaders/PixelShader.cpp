#include "pch.h"
#include "PixelShader.h"

Engine::PixelShader::PixelShader()
{
}

Engine::PixelShader::~PixelShader()
{
}

ID3D11PixelShader* Engine::PixelShader::GetPixelShader()
{
	return myPixelShader.Get();
}
