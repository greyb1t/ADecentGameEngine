#include "pch.h"
#include "Directx11Framework.h"
#include "WindowHandler.h"
#include "Utils/DxUtils.h"
#include "Texture/FullscreenTextureFactory.h"
#include "TracyProfilingGPU.h"

namespace Engine
{
	Directx11Framework::Directx11Framework(WindowHandler& aWindowHandler)
		: myWindowHandler(aWindowHandler)
	{
		aWindowHandler.AddObserver(EventType::WindowResized, this);
	}

	Directx11Framework::~Directx11Framework()
	{
		myWindowHandler.RemoveObserver(EventType::WindowResized, this);

		DestroyTracyGPUProfiling();
	}

	bool Directx11Framework::Init()
	{
		DXGI_SWAP_CHAIN_DESC swapChainDesc = { };
		{
			swapChainDesc.BufferCount = 1;
			swapChainDesc.BufferDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM/*DXGI_FORMAT_R16G16B16A16_FLOAT*/;
			swapChainDesc.BufferUsage =
				DXGI_USAGE_RENDER_TARGET_OUTPUT |
				DXGI_USAGE_SHADER_INPUT /* Required to be able to make ShaderResourceView of it */;
			swapChainDesc.OutputWindow = myWindowHandler.GetWindowHandle();
			swapChainDesc.SampleDesc.Count = 1;
			swapChainDesc.Windowed = true; // NOTE: Windowed to TRUE by default is recommended on MSDN SetFullcreenState()

			// For fullscreen switch
			// swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		}

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
			LOG_ERROR(LogType::Renderer) << "D3D11CreateDeviceAndSwapChain failed " << result;
			return false;
		}

		CreateTracyGPUProfiling(myDevice.Get(), myContext.Get());

		//if (!CheckFeatures())
		//{
		//	LOG_ERROR(LogType::Renderer) << "Unsupported graphics card" << result;
		//	return false;
		//}

		// result = myDevice->CreateDeferredContext(0, &myDeferredContext);
		// 
		// if (FAILED(result))
		// {
		// 	LOG_ERROR(LogType::Renderer) << "CreateDeferredContext failed, " << result << result;
		// 	return false;
		// }

		InitBackbuffer();

		myHasInited = true;

		return true;
	}

	void Directx11Framework::BeginFrame(const std::array<float, 4>& aClearColor)
	{
		aClearColor;

		myBackBufferTexture->ClearTexture();
		myBackBufferTexture2->ClearTexture();

		//myContext->ClearRenderTargetView(myBackBuffer, aClearColor.data());
		//myContext->ClearDepthStencilView(myDepthBuffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
	}

	void Directx11Framework::EndFrame()
	{
		if (myVsync)
		{
			mySwapChain->Present(1, 0);
		}
		else
		{
			mySwapChain->Present(0, 0);
		}

		CollectTracyGPUProfiling();
	}

	ID3D11Device* Directx11Framework::GetDevice() const
	{
		return myDevice.Get();
	}

	ID3D11DeviceContext* Directx11Framework::GetContext() const
	{
		return myContext.Get();
	}

	FullscreenTexture& Directx11Framework::GetBackBuffer() const
	{
		return *myBackBufferTexture;
		//return myBackBufferRTV.Get();
	}

	Engine::FullscreenTexture& Directx11Framework::GetBackBuffer2() const
	{
		return *myBackBufferTexture2;
	}

	IDXGISwapChain& Directx11Framework::GetSwapchain()
	{
		return *mySwapChain.Get();
	}

	const Engine::WindowHandler& Directx11Framework::GetWindowHandler() const
	{
		return myWindowHandler;
	}

	void Directx11Framework::SetVSync(bool aState)
	{
		myVsync = aState;
	}

	void Directx11Framework::Receive(const EventType aEventType, const std::any& aValue)
	{
		if (aEventType == EventType::WindowResized)
		{
			OnResized(std::any_cast<WindowResizedData>(aValue));
		}
	}

	void Directx11Framework::OnResized(const WindowResizedData& aResizeData)
	{
		// When we initially set to fullscreen when creating the swap chain
		// a resize window event comes and crashes because nothing is
		// yet initialized
		if (!myHasInited)
		{
			return;
		}

		// ONLY NEEDED BECAUSE THERE IS A union IN THIS CLASS THAT CANNOT HAVE COMPTR
		//myBackBufferTexture->Release();

		// Release the ComPtrs in the texture
		myBackBufferTexture = nullptr;

		// myBackBufferTexture2->Release();

		// Release the ComPtrs in the texture
		myBackBufferTexture2 = nullptr;

		myContext->ClearState();
		myContext->Flush();


		HRESULT result = mySwapChain->ResizeBuffers(
			0 /* or 2? */,
			aResizeData.myClientSize.x,
			aResizeData.myClientSize.y,
			DXGI_FORMAT_UNKNOWN,
			0);

		std::string message = std::system_category().message(result);

		assert(SUCCEEDED(result));

		InitBackbuffer();
	}

	bool Directx11Framework::InitBackbuffer()
	{
		// ID3D11Texture2D* backBufferTexture = nullptr;
		ComPtr<ID3D11Texture2D> backBufferTexture;

		HRESULT result = mySwapChain->GetBuffer(
			0,
			__uuidof(ID3D11Texture2D),
			reinterpret_cast<void**>(backBufferTexture.GetAddressOf()));

		if (FAILED(result))
		{
			LOG_ERROR(LogType::Renderer) << "GetBuffer(backbuffer) failed " << result;

			return false;
		}

		D3D11_TEXTURE2D_DESC dd;
		backBufferTexture->GetDesc(&dd);

		// TODO: why do I create another factory here?
		FullscreenTextureFactory fullscreenTextureFactory;
		fullscreenTextureFactory.Init(myDevice.Get(), myContext.Get());

		myBackBufferTexture = MakeOwned<FullscreenTexture>(
			fullscreenTextureFactory.CreateTexture(backBufferTexture.Get()));

		const auto renderSizeui = myWindowHandler.GetRenderingSize();

		myBackBufferTexture2 = MakeOwned<FullscreenTexture>(
			fullscreenTextureFactory.CreateTexture(renderSizeui, DXGI_FORMAT_R8G8B8A8_UNORM));

		return true;
	}

	bool Directx11Framework::CheckFeatures()
	{
		D3D11_FEATURE_DATA_THREADING threadingFeature;

		HRESULT result = myDevice->CheckFeatureSupport(
			D3D11_FEATURE_THREADING,
			&threadingFeature,
			sizeof(D3D11_FEATURE_DATA_THREADING));

		if (SUCCEEDED(result))
		{
			// if true, we can create more than 1 resource at a time
			if (threadingFeature.DriverConcurrentCreates == FALSE)
			{
				LOG_ERROR(LogType::Engine) << "Graphics card does not support creating resources on multiple threads";
				return false;
			}

			if (threadingFeature.DriverCommandLists == FALSE)
			{
				LOG_ERROR(LogType::Engine) << "Graphics card does not support command lists";
				return false;
			}
		}

		return true;
	}

}