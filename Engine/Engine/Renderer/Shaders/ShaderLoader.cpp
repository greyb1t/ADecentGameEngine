#include "pch.h"
#include "ShaderLoader.h"
#include "Engine/Renderer/Shaders/VertexShader.h"
#include "Engine/Renderer/Shaders/PixelShader.h"
#include "Engine/Renderer/Shaders/GeometryShader.h"

namespace Engine
{
#ifdef _RETAIL
	const static UINT ShaderCompileFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#else
	const static UINT ShaderCompileFlags =
		D3DCOMPILE_DEBUG |
		D3DCOMPILE_SKIP_OPTIMIZATION |
		D3DCOMPILE_WARNINGS_ARE_ERRORS;
#endif

	// Features
	// * Searches local directory first
	// * Searches base directory secondly
	// * Avoids multiple includes of same .hlsli files
	class MyIncludeHandler : public ID3DInclude
	{
	public:
		HRESULT Open(
			D3D_INCLUDE_TYPE aIncludeType,
			LPCSTR aFileName,
			LPCVOID aParentData,
			LPCVOID* aData,
			UINT* aBytes) override
		{
			if (aIncludeType != D3D_INCLUDE_TYPE::D3D_INCLUDE_LOCAL)
			{
				LOG_ERROR(LogType::Renderer) << "Unhandled include type";
				return E_FAIL;
			}

			std::string fullPath = myIncludeSearchPath + std::string(aFileName);

			// Search the base path as well
			if (!std::filesystem::exists(fullPath))
			{
				//fullPath = "Assets\\Shaders\\Runtime\\" + std::string(aFileName);
				fullPath = myBaseShaderPath->string() + std::string(aFileName);
			}

			const Path absolutePath = Path(std::filesystem::absolute(fullPath));

			bool alreadyIncluded = false;

			if (std::find(
				myIncludedPaths.begin(),
				myIncludedPaths.end(),
				absolutePath) != myIncludedPaths.end())
			{
				alreadyIncluded = true;
			}

			if (alreadyIncluded)
			{
				*aData = nullptr;
				*aBytes = 0;

				return S_OK;
			}
			else
			{
				myIncludedPaths.push_back(absolutePath);
			}

			std::ifstream file(fullPath, std::ios::binary);

			if (!file.is_open())
			{
				LOG_ERROR(LogType::Renderer) << "Cannot open include file " << aFileName << "SearchPath: " << fullPath;
				return E_FAIL;
			}

			file.seekg(0, file.end);
			const auto length = file.tellg();
			file.seekg(0, file.beg);

			myFileDataBuffer.resize(length);

			file.read(reinterpret_cast<char*>(myFileDataBuffer.data()), length);

			*aData = myFileDataBuffer.data();
			*aBytes = length;

			myIncludedFilePaths.push_back(fullPath);

			return S_OK;
		}

		HRESULT Close(LPCVOID pData) override
		{
			myFileDataBuffer.clear();

			return S_OK;
		}

		void PrepareForCompilation(const std::string& aIncludeSearchPath,
			std::filesystem::path* aBaseShaderPath)
		{
			myIncludeSearchPath = aIncludeSearchPath;
			myIncludedFilePaths.clear();
			myIncludedPaths.clear();
			myBaseShaderPath = aBaseShaderPath;
		}

		const std::vector<std::string>& GetIncludedFilePaths() const
		{
			return myIncludedFilePaths;
		}

	private:
		std::string myIncludeSearchPath;
		std::filesystem::path* myBaseShaderPath = nullptr;

		std::vector<uint8_t> myFileDataBuffer;
		std::vector<std::string> myIncludedFilePaths;

		std::vector<Path> myIncludedPaths;
	};
}

Engine::ShaderLoader::ShaderLoader()
{
}

Engine::ShaderLoader::~ShaderLoader()
{
}

bool Engine::ShaderLoader::Init(
	ID3D11Device* aDevice,
	const std::filesystem::path& aBaseShaderPath)
{
	myIncludeHandler = MakeOwned<MyIncludeHandler>();
	myDevice = aDevice;
	myBaseShaderPath = aBaseShaderPath;

	return true;
}

Owned<Engine::VertexShader> Engine::ShaderLoader::LoadVertexShader(
	const std::string& aPath,
	const D3D11_INPUT_ELEMENT_DESC* aElements,
	const int aElementCount)
{
	HRESULT result = 0;

	std::ifstream vertexShaderFile(aPath, std::ios::binary);

	if (!vertexShaderFile.is_open())
	{
		LOG_ERROR(LogType::Renderer) << "Failed to open " << aPath;
		return nullptr;
	}

	ID3D11VertexShader* vertexShader = nullptr;

	std::vector<unsigned char> vertexShaderBuffer(std::istreambuf_iterator<char>(vertexShaderFile), { });
	result = myDevice->CreateVertexShader(vertexShaderBuffer.data(), vertexShaderBuffer.size(), nullptr, &vertexShader);

	if (FAILED(result))
	{
		LOG_ERROR(LogType::Renderer) << "CreateVertexShader failed";
		return nullptr;
	}

	vertexShaderFile.close();

	ID3D11InputLayout* inputLayout = nullptr;

	if (aElementCount > 0)
	{
		result = myDevice->CreateInputLayout(
			aElements,
			aElementCount,
			vertexShaderBuffer.data(),
			vertexShaderBuffer.size(),
			&inputLayout);

		if (FAILED(result))
		{
			LOG_ERROR(LogType::Renderer) << "Failed to create input layout for vertex shader";
			return nullptr;
		}
	}

	ComPtr<ID3D11ShaderReflection> reflection;

	result = D3DReflect(
		vertexShaderBuffer.data(),
		vertexShaderBuffer.size(),
		IID_ID3D11ShaderReflection,
		reinterpret_cast<void**>(reflection.GetAddressOf()));

	if (FAILED(result))
	{
		LOG_ERROR(LogType::Renderer) << "D3DReflect failed " << result;
		return nullptr;
	}

	auto vertexShaderResult = MakeOwned<VertexShader>();

	vertexShaderResult->myVertexShader = vertexShader;
	vertexShaderResult->myVertexShaderBuffer = vertexShaderBuffer;
	vertexShaderResult->myInputLayout = inputLayout;
	vertexShaderResult->myReflection = reflection;

	return vertexShaderResult;
}

Owned<Engine::PixelShader> Engine::ShaderLoader::LoadPixelShader(const std::string& aPath)
{
	std::ifstream pixelShaderFile(aPath, std::ios::binary);

	if (!pixelShaderFile.is_open())
	{
		LOG_ERROR(LogType::Renderer) << "Failed to open " << aPath;
		return nullptr;
	}

	ID3D11PixelShader* pixelShader = nullptr;

	std::vector<unsigned char> pixelShaderBuffer(std::istreambuf_iterator<char>(pixelShaderFile), { });
	HRESULT result = myDevice->CreatePixelShader(pixelShaderBuffer.data(), pixelShaderBuffer.size(), nullptr, &pixelShader);

	if (FAILED(result))
	{
		LOG_ERROR(LogType::Renderer) << "CreatePixelShader failed";
		return nullptr;
	}

	pixelShaderFile.close();

	ComPtr<ID3D11ShaderReflection> reflection;

	result = D3DReflect(
		pixelShaderBuffer.data(),
		pixelShaderBuffer.size(),
		IID_ID3D11ShaderReflection,
		reinterpret_cast<void**>(reflection.GetAddressOf()));

	if (FAILED(result))
	{
		LOG_ERROR(LogType::Renderer) << "D3DReflect failed " << result;
		return nullptr;
	}

	auto pixelShaderResult = MakeOwned<PixelShader>();

	pixelShaderResult->myPixelShader = pixelShader;
	pixelShaderResult->myReflection = reflection;

	return pixelShaderResult;
}

Owned<Engine::GeometryShader> Engine::ShaderLoader::LoadGeometryShader(const std::string& aPath)
{
	// Geometry shader
	ID3D11GeometryShader* geometryShader = nullptr;

	std::ifstream geometryShaderFile(aPath, std::ios::binary);

	if (!geometryShaderFile.is_open())
	{
		LOG_ERROR(LogType::Renderer) << "Failed to open " << aPath;
		return nullptr;
	}

	std::vector<unsigned char> geometryShaderBuffer(std::istreambuf_iterator<char>(geometryShaderFile), { });

	HRESULT result = myDevice->CreateGeometryShader(
		geometryShaderBuffer.data(),
		geometryShaderBuffer.size(),
		nullptr,
		&geometryShader);

	if (FAILED(result))
	{
		LOG_ERROR(LogType::Renderer) << "CreateGeometryShader failed";
		return nullptr;
	}

	geometryShaderFile.close();

	ComPtr<ID3D11ShaderReflection> reflection;

	result = D3DReflect(
		geometryShaderBuffer.data(),
		geometryShaderBuffer.size(),
		IID_ID3D11ShaderReflection,
		reinterpret_cast<void**>(reflection.GetAddressOf()));

	if (FAILED(result))
	{
		LOG_ERROR(LogType::Renderer) << "D3DReflect failed " << result;
		return nullptr;
	}

	auto geometryShaderResult = MakeOwned<GeometryShader>();

	geometryShaderResult->myGeometryShader = geometryShader;
	geometryShaderResult->myReflection = reflection;

	return geometryShaderResult;
}

Owned<Engine::PixelShader> Engine::ShaderLoader::CompilePixelShader(
	const std::string& aPath,
	const std::vector<ShaderDefines>& aDefines,
	std::vector<std::string>& aIncludedFilePathsOut)
{
	ComPtr<ID3D11ShaderReflection> refl;
	ComPtr<ID3DBlob> compiledCodeBlob;

	if (!CompileShader(
		aPath,
		"PSMain",
		"ps_5_0",
		aDefines,
		aIncludedFilePathsOut,
		refl,
		compiledCodeBlob))
	{
		return nullptr;
	}

	ID3D11PixelShader* pixelShader = nullptr;

	HRESULT result = myDevice->CreatePixelShader(
		compiledCodeBlob->GetBufferPointer(),
		compiledCodeBlob->GetBufferSize(),
		nullptr,
		&pixelShader);

	if (FAILED(result))
	{
		LOG_ERROR(LogType::Renderer) << "CreatePixelShader failed " << result;
		return nullptr;
	}

	auto pixelShaderResult = MakeOwned<PixelShader>();

	pixelShaderResult->myPixelShader = pixelShader;
	pixelShaderResult->myReflection = refl;

	return pixelShaderResult;
}

Owned<Engine::VertexShader> Engine::ShaderLoader::CompileVertexShader(
	const std::string& aPath,
	const D3D11_INPUT_ELEMENT_DESC* aElements,
	const int aElementCount,
	const std::vector<ShaderDefines>& aDefines,
	std::vector<std::string>& aIncludedFilePathsOut)
{
	ComPtr<ID3D11ShaderReflection> refl;
	ComPtr<ID3DBlob> compiledCodeBlob;

	if (!CompileShader(
		aPath,
		"VSMain",
		"vs_5_0",
		aDefines,
		aIncludedFilePathsOut,
		refl,
		compiledCodeBlob))
	{
		return nullptr;
	}

	ID3D11VertexShader* vertexShader = nullptr;

	HRESULT result = myDevice->CreateVertexShader(
		compiledCodeBlob->GetBufferPointer(),
		compiledCodeBlob->GetBufferSize(),
		nullptr,
		&vertexShader);

	if (FAILED(result))
	{
		LOG_ERROR(LogType::Renderer) << "CreateVertexShader failed " << result;
		return nullptr;
	}

	ID3D11InputLayout* inputLayout = nullptr;

	if (aElementCount > 0)
	{
		result = myDevice->CreateInputLayout(
			aElements,
			aElementCount,
			compiledCodeBlob->GetBufferPointer(),
			compiledCodeBlob->GetBufferSize(),
			&inputLayout);

		if (FAILED(result))
		{
			LOG_ERROR(LogType::Renderer) << "Failed to create input layout for vertex shader";
			return nullptr;
		}
	}

	auto vertexShaderResult = MakeOwned<VertexShader>();

	vertexShaderResult->myVertexShader = vertexShader;
	vertexShaderResult->myVertexShaderBuffer = std::vector<uint8_t>(
		reinterpret_cast<uint8_t*>(compiledCodeBlob->GetBufferPointer()),
		reinterpret_cast<uint8_t*>(compiledCodeBlob->GetBufferPointer()) + compiledCodeBlob->GetBufferSize());
	vertexShaderResult->myInputLayout = inputLayout;
	vertexShaderResult->myReflection = refl;

	return vertexShaderResult;
}

Owned<Engine::GeometryShader> Engine::ShaderLoader::CompileGeometryShader(
	const std::string& aPath,
	const std::vector<ShaderDefines>& aDefines,
	std::vector<std::string>& aIncludedFilePathsOut)
{
	ComPtr<ID3D11ShaderReflection> refl;
	ComPtr<ID3DBlob> compiledCodeBlob;

	if (!CompileShader(
		aPath,
		"GSMain",
		"gs_5_0",
		aDefines,
		aIncludedFilePathsOut,
		refl,
		compiledCodeBlob))
	{
		return nullptr;
	}

	ID3D11GeometryShader* geometryShader = nullptr;

	HRESULT result = myDevice->CreateGeometryShader(
		compiledCodeBlob->GetBufferPointer(),
		compiledCodeBlob->GetBufferSize(),
		nullptr,
		&geometryShader);

	if (FAILED(result))
	{
		LOG_ERROR(LogType::Renderer) << "CreateGeometryShader failed " << result;
		return nullptr;
	}

	auto geometryShaderResult = MakeOwned<GeometryShader>();

	geometryShaderResult->myGeometryShader = geometryShader;
	geometryShaderResult->myReflection = refl;

	return geometryShaderResult;
}

bool Engine::ShaderLoader::CompileShaderToBytecode(
	const Path& aPath,
	const std::string& aMainFunctionName,
	const std::string& aTargetShader,
	const UINT aCompileFlags,
	std::vector<uint8_t>& aResultBytecode)
{
	std::ifstream pixelShaderFile(aPath.ToWString(), std::ios::binary);

	if (!pixelShaderFile.is_open())
	{
		LOG_ERROR(LogType::Renderer) << "Failed to open " << aPath;
		return false;
	}

	std::vector<unsigned char> pixelShaderBuffer(std::istreambuf_iterator<char>(pixelShaderFile), { });

	// Create pixel shader
	ComPtr<ID3DBlob> compiledCodeBlob;
	ComPtr<ID3DBlob> errorMessagesBlob;

	myIncludeHandler->PrepareForCompilation(GetIncludeSearchPath(aPath.ToString()).c_str(), &myBaseShaderPath);

	// NOTE: When D3D_COMPILE_STANDARD_FILE_INCLUDE is used, the 3rd argument pSourceName
	// has to specify the path to the folder where it searches for includes
	HRESULT result = D3DCompile(
		pixelShaderBuffer.data(),
		pixelShaderBuffer.size(),
		NULL,
		NULL,
		myIncludeHandler.get() /*D3D_COMPILE_STANDARD_FILE_INCLUDE*/ /* TODO: handle #include in hlsl here, read MSDN */,
		aMainFunctionName.c_str(),
		aTargetShader.c_str(),
		aCompileFlags,
		0,
		&compiledCodeBlob,
		&errorMessagesBlob);

	pixelShaderFile.close();

	if (FAILED(result))
	{
		const int errorBufferSize = 1024;
		char errorBuffer[errorBufferSize + 1] = { };

		memcpy(
			errorBuffer,
			errorMessagesBlob->GetBufferPointer(),
			std::min(static_cast<int>(errorMessagesBlob->GetBufferSize()), errorBufferSize));

		LOG_ERROR(LogType::Renderer) << "Compiling: " << aPath << ", D3DCompile error: " << errorBuffer;

		return false;
	}

	aResultBytecode.clear();
	aResultBytecode.insert(
		aResultBytecode.begin(),
		reinterpret_cast<uint8_t*>(compiledCodeBlob->GetBufferPointer()),
		reinterpret_cast<uint8_t*>(
			reinterpret_cast<uintptr_t>(compiledCodeBlob->GetBufferPointer()) + compiledCodeBlob->GetBufferSize()));

	return true;
}

std::string Engine::ShaderLoader::GetIncludeSearchPath(const std::string& aShaderPath) const
{
	const size_t lastSlash = aShaderPath.find_last_of('\\');

	std::string pathToFolder;

	if (lastSlash == std::string::npos)
	{
		pathToFolder = "\\";
	}
	else
	{
		pathToFolder = aShaderPath.substr(0, lastSlash + 1);
	}

	return pathToFolder;
}

bool Engine::ShaderLoader::CompileShader(
	const Path& aPath,
	const std::string& aMainFunctionName,
	const std::string& aTargetName,
	const std::vector<ShaderDefines>& aDefines,
	std::vector<std::string>& aIncludedFilePathsOut,
	ComPtr<ID3D11ShaderReflection>& aReflectionOut,
	ComPtr<ID3DBlob>& aCompiledCodeBlobOut)
{
	std::ifstream geometryShaderFile(aPath.ToWString(), std::ios::binary);

	if (!geometryShaderFile.is_open())
	{
		LOG_ERROR(LogType::Renderer) << "Failed to open " << aPath;
		return false;
	}

	std::vector<unsigned char> geometryShaderBuffer(std::istreambuf_iterator<char>(geometryShaderFile), { });

	// Create pixel shader
	// ComPtr<ID3DBlob> compiledCodeBlob;
	ComPtr<ID3DBlob> errorMessagesBlob;

	myIncludeHandler->PrepareForCompilation(
		GetIncludeSearchPath(aPath.ToString()),
		&myBaseShaderPath);

	D3D_SHADER_MACRO defines[static_cast<int>(ShaderDefines::Count) + 1] = { };

	for (int i = 0; i < aDefines.size(); ++i)
	{
		defines[i] = { ShaderDefinesStrings[static_cast<int>(aDefines[i])].c_str() };
	}

	// D3D_SHADER_MACRO test;
	// test.

	// NOTE: When D3D_COMPILE_STANDARD_FILE_INCLUDE is used, the 3rd argument pSourceName
	// has to specify the path to the folder where it searches for includes
	HRESULT result = D3DCompile(
		geometryShaderBuffer.data(),
		geometryShaderBuffer.size(),
		NULL,
		defines,
		myIncludeHandler.get(),
		aMainFunctionName.c_str(),
		aTargetName.c_str(),
		ShaderCompileFlags,
		0,
		&aCompiledCodeBlobOut,
		&errorMessagesBlob);

	geometryShaderFile.close();

	aIncludedFilePathsOut = myIncludeHandler->GetIncludedFilePaths();

	if (FAILED(result))
	{
		const int errorBufferSize = 1024;
		char errorBuffer[errorBufferSize + 1] = { };

		memcpy(
			errorBuffer,
			errorMessagesBlob->GetBufferPointer(),
			std::min(static_cast<int>(errorMessagesBlob->GetBufferSize()), errorBufferSize));

		LOG_ERROR(LogType::Renderer) << "Compiling: " << aPath << ", D3DCompile error: " << errorBuffer;

		return false;
	}

	result = D3DReflect(
		aCompiledCodeBlobOut->GetBufferPointer(),
		aCompiledCodeBlobOut->GetBufferSize(),
		IID_ID3D11ShaderReflection,
		reinterpret_cast<void**>(aReflectionOut.GetAddressOf()));

	if (FAILED(result))
	{
		LOG_ERROR(LogType::Renderer) << "D3DReflect failed " << result;
		return false;
	}

	return true;
}
