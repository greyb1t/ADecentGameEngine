#pragma once

#include "Shader.h"

struct ID3D11GeometryShader;

namespace Engine
{
	// CURRENT JUST A SIMPLE WRAPPER, WILL BE MORE LATER
	class GeometryShader : public Shader
	{
	public:
		GeometryShader();
		~GeometryShader();

		ID3D11GeometryShader* GetGeometryShader();

	private:
		friend class ShaderLoader;

		ComPtr<ID3D11GeometryShader> myGeometryShader = nullptr;
	};
}