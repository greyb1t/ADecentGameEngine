#pragma once

#include "Utils\Observer.h"

namespace Engine
{
	class WindowHandler;
	class FullscreenTexture;

	class Directx11Framework : public Observer
	{
	public:
		Directx11Framework(WindowHandler& aWindowHandler);
		~Directx11Framework();

		bool Init();

		void BeginFrame(const std::array<float, 4>& aClearColor);
		void EndFrame();

		ID3D11Device* GetDevice() const;
		ID3D11DeviceContext* GetContext() const;
		FullscreenTexture& GetBackBuffer() const;
		FullscreenTexture& GetBackBuffer2() const;
		IDXGISwapChain& GetSwapchain();

		const WindowHandler& GetWindowHandler() const;

		void SetVSync(bool aState);

		void Receive(const EventType aEventType, const std::any& aValue) override;

		static ID3D11Device& GetDevice2() { return *myDevice.Get(); }

	private:
		void OnResized(const WindowResizedData& aResizeData);

		bool InitBackbuffer();
		bool CheckFeatures();

	private:
		bool myHasInited = false;
		bool myVsync = false;

		static inline ComPtr<ID3D11Device> myDevice;
		ComPtr<ID3D11DeviceContext> myContext;
		ComPtr<IDXGISwapChain> mySwapChain;

		// ComPtr<ID3D11DeviceContext> myDeferredContext;

		Owned<FullscreenTexture> myBackBufferTexture;
		Owned<FullscreenTexture> myBackBufferTexture2;

		WindowHandler& myWindowHandler;
	};
}