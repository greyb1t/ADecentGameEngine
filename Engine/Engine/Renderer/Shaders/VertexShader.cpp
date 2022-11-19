#include "pch.h"
#include "VertexShader.h"

Engine::VertexShader::VertexShader()
{
}

Engine::VertexShader::~VertexShader()
{
}

ID3D11VertexShader* Engine::VertexShader::GetVertexShader()
{
	return myVertexShader.Get();
}

const std::vector<uint8_t>& Engine::VertexShader::GetBuffer() const
{
	return myVertexShaderBuffer;
}

ID3D11InputLayout* Engine::VertexShader::GetInputLayout()
{
	return myInputLayout;
}
