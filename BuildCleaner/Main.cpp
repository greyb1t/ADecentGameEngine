#pragma once

#include <d3d11.h>
#include <assert.h>
#include <stdint.h>
#include <string>
#include <Windows.h>
#include <iostream>
#include <d3dcompiler.h>
#include <d3d11shader.h>
#include <filesystem>

// DirectXTK
#include <DirectXTK/DDSTextureLoader.h>

// Common
#include "Common/Types.h"
#include "Common/Path.h"
#include "Common/Math/Math.h"

namespace C = Common;

using Vec2ui = Common::Vector2ui;
using Vec2f = Common::Vector2f;
using Vec3f = Common::Vector3f;
using Vec4f = Common::Vector4f;
using Quatf = Common::Quaternion;
using Mat4f = Common::Matrix4f;
using Mat3f = Common::Matrix3f;

/*
	ComPtr Best Pratices:
	Reference: https://github.com/microsoft/DirectXTK/wiki/ComPtr

	* When taking array of pointers user .GetAddressOf()

	* When building array of d3d interface pointers, build one as local variable
		- ID3D11SamplerState* samplers[] = { sampler1.Get(), sampler2.Get() };

	* If created fresh ComPtr and using a factory, can use: .GetAddressOf()
		-	hr = D3D11CreateDevice( ..., device.GetAddressOf(), ... );
			or
			hr = D3D11CreateDevice( ..., &device, ... );

	* Passing ComPtr as by value increment/decrements the reference count
		Avoid by passing by const& or raw pointer. Raw pointers are more flexible.

	* Can use .Reset() to release reference.

	* Check for null: by if (!SomeComPtrHere) { it is nullptr }


*/
#include <wrl/client.h>
using namespace Microsoft::WRL;
//
//// Assimp
//#include <assimp/mesh.h>
//#include <assimp/Importer.hpp>
//#include <assimp/postprocess.h>
//#include <assimp/scene.h>
//#include "assimp\quaternion.h"
//#include "assimp\vector3.h"
//#include "assimp\matrix4x4.h"
//#include "assimp\anim.h"
//
//// libdeflate
//#include <libdeflate/libdeflate.h>

#include "DirectXTK/DDSTextureLoader.h"

#pragma comment(lib, "d3d11.lib")
#ifdef _DEBUG
#pragma comment(lib, "DirectXTK-debug.lib")
#else
#pragma comment(lib, "DirectXTK-release.lib")
#endif

bool AllowedToMakeRetailHere()
{
	for (const auto& folder : std::filesystem::current_path())
	{
		if (folder.string() == "GameProject")
		{
			return false;
		}
	}

	return true;
}

void RemoveAllFilesWithExtension(const std::string& aExtension)
{
	for (auto& dir : std::filesystem::recursive_directory_iterator(std::filesystem::current_path()))
	{
		if (dir.path().has_extension())
		{
			Path p = dir.path().string();

			if (p.IsExtension(aExtension))
			{
				// because they are read only
				std::filesystem::permissions(
					dir.path(), std::filesystem::perms::all, std::filesystem::perm_options::add);

				std::error_code e;
				std::filesystem::remove(dir.path(), e);

				if (e.value() != 0)
				{
					std::cout << "fs::remove failed with: " << e.message() << " path: " << p
							  << std::endl;
				}
			}
		}
	}
}

static bool IsPowerOf2(int aN)
{
	return (aN & (aN - 1)) == 0;
}

C::Vector2ui GetTextureSize(ID3D11ShaderResourceView& aSrv)
{
	ID3D11Resource* resource = nullptr;
	aSrv.GetResource(&resource);
	ID3D11Texture2D* textureResource = reinterpret_cast<ID3D11Texture2D*>(resource);
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureResource->GetDesc(&textureDesc);

	C::Vector2ui textureSize = C::Vector2ui(textureDesc.Width, textureDesc.Height);

	textureResource->Release();

	return textureSize;
}

bool WarningAboutNotPowerOf2(ID3D11Device& aDevice)
{
	bool result = true;

	for (auto& dir : std::filesystem::recursive_directory_iterator(std::filesystem::current_path()))
	{
		Path p = dir.path().string();

		if (dir.path().has_extension())
		{
			if (p.IsExtension(".dds"))
			{
				ComPtr<ID3D11Texture2D> myTexture2D = nullptr;
				ComPtr<ID3D11ShaderResourceView> mySRV = nullptr;

				HRESULT hr = DirectX::CreateDDSTextureFromFile(
					&aDevice, p.ToWString().c_str(), nullptr, &mySRV);

				if (SUCCEEDED(hr))
				{
					const auto textureSize = GetTextureSize(*mySRV.Get());

					if (!IsPowerOf2(textureSize.x) || !IsPowerOf2(textureSize.y))
					{
						p.MakeRelativeTo(Path(std::filesystem::current_path()));

						std::cout << p << " is not power of 2 (Size: x: " << std::dec
								  << textureSize.x << ", y: " << textureSize.y << ")" << std::endl;

						result = false;
					}
				}
				else
				{
					std::cout << "Failed to open: " << p
							  << " is it bcuz of not being a proper size?, hr: " << std::hex << hr
							  << std::endl;
				}
			}
		}
	}

	return result;
}

bool TryRemovingWholeFolder(const std::filesystem::path& aPath)
{
	if (!std::filesystem::exists(aPath))
	{
		return true;
	}

	for (auto& dir : std::filesystem::recursive_directory_iterator(aPath))
	{
		if (dir.path().has_extension())
		{
			// because they are read only
			std::filesystem::permissions(
				dir.path(), std::filesystem::perms::all, std::filesystem::perm_options::add);

			std::error_code e;
			std::filesystem::remove(dir.path(), e);

			if (e.value() != 0)
			{
				std::cout << "fs::remove failed with: " << e.message() << " path: " << dir.path()
						  << std::endl;
			}
		}
	}

	// because they are read only
	std::filesystem::permissions(
		aPath, std::filesystem::perms::all, std::filesystem::perm_options::add);

	std::error_code e;
	std::filesystem::remove_all(aPath, e);

	if (e.value() != 0)
	{
		std::cout << "std::remove_all failed with: " << e.message() << " in path " << aPath
				  << std::endl;
		return false;
	}

	return true;
}

bool RemoveEmptyFolders()
{
	std::vector<std::filesystem::path> foldersToRemove;

	// do this until we found no folers to remove
	// because if a folder contains anotther empty folder its not
	// considered empty, therefore, only the deepest gets removed
	do
	{
		foldersToRemove.clear();

		for (auto& dir :
			std::filesystem::recursive_directory_iterator(std::filesystem::current_path()))
		{
			if (!dir.path().has_extension())
			{
				if (std::filesystem::is_empty(dir.path()))
				{
					foldersToRemove.push_back(dir.path());
				}
			}
		}

		for (const auto& path : foldersToRemove)
		{
			if (!TryRemovingWholeFolder(path))
			{
				return false;
			}
		}

	} while (!foldersToRemove.empty());

	return true;
}

bool MakeRetailBuild(ID3D11Device& aDevice)
{
	bool result = true;

	std::cout << "Started retail build" << std::endl;

	// Remove Runtime shaders
	// std::cout << "Removing runtime shaders" << std::endl;
	// if (!TryRemovingWholeFolder(std::filesystem::current_path() / "Assets\\Shaders\\Runtime"))
	// {
	// 	result = false;
	// }

	std::cout << "Removing all .ps files" << std::endl;
	RemoveAllFilesWithExtension(".ps");

	std::cout << "Removing all .vs files" << std::endl;
	RemoveAllFilesWithExtension(".vs");

	std::cout << "Removing all .gs files" << std::endl;
	RemoveAllFilesWithExtension(".gs");

	// Remove all .fbx
	std::cout << "Removing all .FBX files" << std::endl;
	RemoveAllFilesWithExtension(".fbx");

	// Remove all .tga
	std::cout << "Removing all .TGA files" << std::endl;
	RemoveAllFilesWithExtension(".tga");

	// Remove all .pdb
	std::cout << "Removing all .PDB files" << std::endl;
	RemoveAllFilesWithExtension(".pdb");

	// Remove all cubemap .hdr files
	// std::cout << "Removing all .HDR files" << std::endl;
	// RemoveAllFilesWithExtension(".hdr");

	std::cout << "Removing all .PNG files" << std::endl;
	RemoveAllFilesWithExtension(".png");

	std::cout << "Removing all .JPG files" << std::endl;
	RemoveAllFilesWithExtension(".jpg");

	// Remove LibrariesDebug
	std::cout << "Removing LibrariesDebug folder" << std::endl;
	if (!TryRemovingWholeFolder(std::filesystem::current_path() / "LibrariesDebug"))
	{
		result = false;
	}

	// Remove Dev folder
	std::cout << "Removing Dev folder" << std::endl;
	if (!TryRemovingWholeFolder(std::filesystem::current_path() / "Dev"))
	{
		result = false;
	}

	if (!WarningAboutNotPowerOf2(aDevice))
	{
		result = false;
	}

	// last step is to remove empty folders
	std::cout << "Removing empty folders" << std::endl;
	if (!RemoveEmptyFolders())
	{
		result = false;
	}

	//// Check for blacklisted filetypes: .png, .jpg
	//std::cout << "Seaching for blacklisted files" << std::endl;
	//const auto blacklistedFilesFound = FindBlacklistedFiles();

	//if (!blacklistedFilesFound.empty())
	//{
	//	for (const auto& filePath : blacklistedFilesFound)
	//	{
	//		std::cout << "BLACKLISTED FILE: " << filePath << std::endl;
	//	}

	//	result = false;
	//}

	return result;
}

int main()
{
	std::cout << "Press any key to continue with the dirty work" << std::endl;

	std::cin.get();

	const wchar_t* className = L"Dx11WindowClass";

	// From Launcher resource.h
	const int iconId = 101;

	WNDCLASS windowClass = {};
	windowClass.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
	windowClass.lpfnWndProc = [](HWND aWindowHandle, UINT aMessage, WPARAM aWparam, LPARAM aLparam)
	{
		return DefWindowProc(aWindowHandle, aMessage, aWparam, aLparam);
	};
	windowClass.hInstance = GetModuleHandle(NULL);
	windowClass.hIcon = NULL;
	windowClass.hCursor = nullptr;
	windowClass.lpszClassName = className;
	windowClass.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));

	const ATOM atom = RegisterClass(&windowClass);

	if (!atom)
	{
		// LOG_ERROR(LogType::Renderer) << "RegisterClass failed " << GetLastError();

		return false;
	}

	LONG_PTR style = WS_VISIBLE;

	HWND myWindowHandle = CreateWindow(
		className,
		L"Money",
		style | WS_VISIBLE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		800,
		600,
		nullptr,
		nullptr,
		reinterpret_cast<HINSTANCE>(GetModuleHandle(NULL)),
		NULL);

	if (!myWindowHandle)
	{
		std::cout << "CreateWindow failed " << GetLastError();

		return false;
	}

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	{
		swapChainDesc.BufferCount = 1;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM/*DXGI_FORMAT_R16G16B16A16_FLOAT*/;
		swapChainDesc.BufferUsage =
			DXGI_USAGE_RENDER_TARGET_OUTPUT |
			DXGI_USAGE_SHADER_INPUT /* Required to be able to make ShaderResourceView of it */;
		swapChainDesc.OutputWindow = myWindowHandle;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.Windowed = true; // NOTE: Windowed to TRUE by default is recommended on MSDN SetFullcreenState()

		// For fullscreen switch
		// swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	}

	ComPtr<ID3D11Device> myDevice;
	ComPtr<ID3D11DeviceContext> myContext;
	ComPtr<IDXGISwapChain> mySwapChain;

	HRESULT result = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		// This flag causes crashed in graphics debugger
		// TODO: Display a red text when debugging is active to avoid
		// having it activated by mistake
		/*D3D11_CREATE_DEVICE_DEBUG*/0,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&mySwapChain,
		&myDevice,
		nullptr,
		&myContext);

	if (FAILED(result))
	{
		// LOG_ERROR(LogType::Renderer) << "D3D11CreateDeviceAndSwapChain failed " << result;
		return false;
	}

	if (AllowedToMakeRetailHere())
	{
		std::cout << "Starting retail build" << std::endl;
		MakeRetailBuild(*myDevice.Get());
	}
	else
	{
		std::cout << "Not allowed to make retail build in perforce repo!" << std::endl;
	}

	std::cout << "Finished" << std::endl;

	int end;
	std::cin >> end;

	return 0;
}