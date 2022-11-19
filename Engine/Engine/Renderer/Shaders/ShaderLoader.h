#pragma once

#include "Shader.h"

namespace Engine
{
	class VertexShader;
	class PixelShader;
	class GeometryShader;
	class MyIncludeHandler;

	class ShaderLoader
	{
	public:
		ShaderLoader();
		~ShaderLoader();

		bool Init(ID3D11Device* aDevice, const std::filesystem::path& aBaseShaderPath);

		Owned<VertexShader> LoadVertexShader(
			const std::string& aPath,
			const D3D11_INPUT_ELEMENT_DESC* aElements,
			const int aElementCount);

		Owned<PixelShader> LoadPixelShader(const std::string& aPath);
		Owned<GeometryShader> LoadGeometryShader(const std::string& aPath);

		Owned<PixelShader> CompilePixelShader(
			const std::string& aPath,
			const std::vector<ShaderDefines>& aDefines,
			std::vector<std::string>& aIncludedFilePathsOut);

		Owned<VertexShader> CompileVertexShader(
			const std::string& aPath,
			const D3D11_INPUT_ELEMENT_DESC* aElements,
			const int aElementCount,
			const std::vector<ShaderDefines>& aDefines,
			std::vector<std::string>& aIncludedFilePathsOut);

		Owned<GeometryShader> CompileGeometryShader(
			const std::string& aPath,
			const std::vector<ShaderDefines>& aDefines,
			std::vector<std::string>& aIncludedFilePathsOut);

		bool CompileShaderToBytecode(
			const Path& aPath,
			const std::string& aMainFunctionName,
			const std::string& aTargetShader,
			const UINT aCompileFlags,
			std::vector<uint8_t>& aResultBytecode);

	private:
		std::string GetIncludeSearchPath(const std::string& aShaderPath) const;

		bool CompileShader(
			const Path& aPath,
			const std::string& aMainFunctionName,
			const std::string& aTargetName,
			const std::vector<ShaderDefines>& aDefines,
			std::vector<std::string>& aIncludedFilePathsOut,
			ComPtr<ID3D11ShaderReflection>& aReflectionOut,
			ComPtr<ID3DBlob>& aCompiledCodeBlobOut);

	private:
		ID3D11Device* myDevice = nullptr;

		std::filesystem::path myBaseShaderPath;

		Owned<MyIncludeHandler> myIncludeHandler;
	};
};