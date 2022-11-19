#include "pch.h"
#include "Shader.h"

ID3D11ShaderReflection* Engine::Shader::GetReflection() const
{
	return myReflection.Get();
}
