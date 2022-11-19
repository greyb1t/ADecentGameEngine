#pragma once

#include "Shader.h"

struct ID3D11PixelShader;

namespace Engine
{
	// CURRENT JUST A SIMPLE WRAPPER, WILL BE MORE LATER
	class PixelShader : public Shader
	{
	public:
		PixelShader();
		~PixelShader();

		ID3D11PixelShader* GetPixelShader();

	private:
		friend class ShaderLoader;

		ComPtr<ID3D11PixelShader> myPixelShader = nullptr;
	};
}