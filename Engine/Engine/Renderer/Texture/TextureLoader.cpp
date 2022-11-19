#include "pch.h"
#include "TextureLoader.h"
#include "Engine/Renderer/Texture/Texture2D.h"
#include "Engine/Paths.h"
#include "Engine/Engine.h"
#include "..\DirectXTex\DirectXTex.h"

Engine::TextureLoader::TextureLoader()
{
}

Engine::TextureLoader::~TextureLoader()
{
}

bool Engine::TextureLoader::Init(ID3D11Device* aDevice, ID3D11DeviceContext& aContext)
{
	myDevice = aDevice;
	myContext = &aContext;

	return true;
}

DWORD Win32FromHResult(HRESULT hr)
{
	if ((hr & 0xFFFF0000) == MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, 0))
	{
		return HRESULT_CODE(hr);
	}

	if (hr == S_OK)
	{
		return ERROR_SUCCESS;
	}

	// Not a Win32 HRESULT so return a generic error code.
	return ERROR_CAN_NOT_COMPLETE;
}

std::string GetLastErrorAsString(DWORD error)
{
	//Get the error message ID, if any.
	DWORD errorMessageID = error;
	if (errorMessageID == 0)
	{
		return std::string(); //No error message has been recorded
	}

	LPSTR messageBuffer = nullptr;

	//Ask Win32 to give us the string version of that message ID.
	//The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	//Copy the error message into a std::string.
	std::string message(messageBuffer, size);

	//Free the Win32's string's buffer.
	LocalFree(messageBuffer);

	return message;
}

bool IsPowerOf2(int aN)
{
	return (aN & (aN - 1)) == 0;
}

Shared<Engine::Texture2D> Engine::TextureLoader::LoadTexture(const std::string& aPath)
{
	if (aPath.find("UI_HoveredFade") != std::string::npos)
	{
		int test = 0;
		// aPath = "Assets/Engine/PlaceholderTextures/PlaceholderCubemap.dds";
	}

	assert(myDevice);

	auto resultTexture = MakeShared<Texture2D>();

	// HRESULT result = DirectX::CreateDDSTextureFromFile(
	// 	myDevice,
	// 	myContext,
	// 	std::wstring(aPath.begin(), aPath.end()).c_str(),
	// 	nullptr /* should I get resource? */,
	// 	&resultTexture->mySRV);

	HRESULT result = DirectX::CreateDDSTextureFromFile(
		myDevice,
		std::wstring(aPath.begin(), aPath.end()).c_str(),
		nullptr,
		&resultTexture->mySRV);

	// myContext->GenerateMips(resultTexture->mySRV.Get());

	if (SUCCEEDED(result))
	{
		ComPtr<ID3D11Resource> resource;
		resultTexture->mySRV.Get()->GetResource(&resource);
		ID3D11Texture2D* textureResource = reinterpret_cast<ID3D11Texture2D*>(resource.Get());
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureResource->GetDesc(&textureDesc);

		Vec2ui textureSize = Vec2ui(textureDesc.Width, textureDesc.Height);

		if ((textureDesc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE) != 0)
		{
			resultTexture->myType = TextureType::Cubemap;
		}
		else
		{
			resultTexture->myType = TextureType::Texture2D;
		}

		//#ifndef _RETAIL
		//		{
		//			ID3D11Resource* resource = nullptr;
		//			resultTexture->mySRV->GetResource(&resource);
		//			ID3D11Texture2D* textureResource = reinterpret_cast<ID3D11Texture2D*>(resource);
		//			D3D11_TEXTURE2D_DESC textureDesc = {};
		//			textureResource->GetDesc(&textureDesc);
		//
		//			if (textureDesc.MipLevels == 1)
		//			{
		//				LOG_WARNING(LogType::Resource) << Path(aPath).GetFilename() << " only has 1 miplevel.";
		//			}
		//
		//			textureResource->Release();
		//		}
		//
		//		if (!IsPowerOf2(textureSize.x) || !IsPowerOf2(textureSize.y))
		//		{
		//			LOG_ERROR(LogType::Renderer) << aPath << " is not power of 2 (Size: x: " <<
		//				textureSize.x << ", y: " << textureSize.y << ")";
		//		}
		//#endif

		resultTexture->mySize = textureSize;

		return resultTexture;
	}

	LOG_ERROR(LogType::Resource) << "CreateDDSTextureFromFile failed " << aPath << " " << GetLastErrorAsString(Win32FromHResult(result));

	//switch (aTextureType)
	//{
	//case TextureType::Cubemap:
	//{
	//	result = DirectX::CreateDDSTextureFromFile(
	//		myDevice,
	//		ourDefaultCubemapPathW.c_str(),
	//		nullptr,
	//		&resultTexture->mySRV);
	//} break;
	//case TextureType::Texture2D:
	//{
	result = DirectX::CreateDDSTextureFromFile(
		myDevice,
		GetEngine().GetPaths().GetPathWString(PathEnum::PlaceholderTexture).c_str(),
		nullptr,
		&resultTexture->mySRV);
	//} break;
	//default:
	//	break;
	//}


	if (SUCCEEDED(result))
	{
		ComPtr<ID3D11Resource> resource;
		resultTexture->mySRV.Get()->GetResource(&resource);
		ID3D11Texture2D* textureResource = reinterpret_cast<ID3D11Texture2D*>(resource.Get());
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureResource->GetDesc(&textureDesc);

		Vec2ui textureSize = Vec2ui(textureDesc.Width, textureDesc.Height);

		if ((textureDesc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE) != 0)
		{
			resultTexture->myType = TextureType::Cubemap;
		}
		else
		{
			resultTexture->myType = TextureType::Texture2D;
		}

		return resultTexture;
	}

	return nullptr;
}

Shared<Engine::Texture2D> Engine::TextureLoader::LoadHDR(const std::string& aPath)
{
	DirectX::TexMetadata metaData = {};
	DirectX::ScratchImage image;

	HRESULT result = DirectX::LoadFromHDRFile(
		std::wstring(aPath.begin(), aPath.end()).c_str(),
		&metaData,
		image);

	if (FAILED(result))
	{
		LOG_ERROR(LogType::Resource) << "Failed to load .HDR: " << aPath;
		return nullptr;
	}

	auto resultTexture = MakeShared<Texture2D>();

	result = DirectX::CreateShaderResourceView(
		myDevice,
		image.GetImages(),
		image.GetImageCount(),
		metaData,
		&resultTexture->mySRV);

	if (FAILED(result))
	{
		LOG_ERROR(LogType::Resource) << "Failed to create .HDR SRV for: " << aPath;
		return nullptr;
	}

	resultTexture->myType = TextureType::HDRCubemap;

	return resultTexture;
}
