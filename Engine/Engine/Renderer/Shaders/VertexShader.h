#pragma once

#include "Shader.h"

struct ID3D11VertexShader;
struct ID3D11InputLayout;

namespace Engine
{
	// CURRENT JUST A SIMPLE WRAPPER, WILL BE MORE LATER
	class VertexShader : public Shader
	{
	public:
		VertexShader();
		~VertexShader();

		ID3D11VertexShader* GetVertexShader();

		const std::vector<uint8_t>& GetBuffer() const;

		ID3D11InputLayout* GetInputLayout();

	private:
		friend class ShaderLoader;

		//ID3D11VertexShader* myVertexShader = nullptr;
		ComPtr<ID3D11VertexShader> myVertexShader = nullptr;
		std::vector<uint8_t> myVertexShaderBuffer;

		// TODO: make comptr
		ID3D11InputLayout* myInputLayout = nullptr;
	};
}