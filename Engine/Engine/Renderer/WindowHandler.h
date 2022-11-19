#pragma once

#include "Utils\Subject.h"
#include "CursorMode.h"

namespace Engine
{
	class Directx11Framework;

	using WndProc_t = std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)>;

	enum WindowFlags
	{
		WindowFlags_None,
		WindowFlags_CalculateRenderingSizeAutomatically = 1 << 0,
	};

	enum class WindowState
	{
		Unset = 0,

		Windowed,
		WindowedBorderless,
		Fullscreen,

		Count
	};

	struct WindowData
	{
		Vec2ui myWindowSize;
		Vec2ui myRenderingSize;
		Vec2ui myTargetSize;
		WindowState myWindowState = WindowState::Windowed;

		HINSTANCE myInstanceHandle = nullptr;
		std::wstring myTitle;
		WndProc_t myWindowProc = nullptr;

		HCURSOR myCursorHandle = nullptr;

		WindowFlags myWindowFlags = WindowFlags::WindowFlags_None;
	};

	class WindowHandler : public Subject
	{
	public:
		WindowHandler() = default;
		~WindowHandler();

		bool Init(const WindowData aWindowData, Directx11Framework& aFramework);

		void Update();

		void SetCursorVisibility(const bool aVisible);
		bool IsCursorVisible() const;

		HCURSOR GetCursorHandle() const;
		void SetCursorHandle(HCURSOR aCursorHandle);

		static LRESULT CALLBACK WndProc(HWND aWindowHandle,
			UINT aMessage, WPARAM aWparam, LPARAM aLparam);

		void SetIsDestroyed(const bool aIsDestroyed);
		bool IsDestroyed() const;

		HWND GetWindowHandle() const;

		template <typename T = unsigned int>
		C::Vector2<T> GetWindowSize() const;

		template <typename T = unsigned int>
		C::Vector2<T> GetRenderingSize() const;

		const Vec2ui& GetTargetSize() const;

		void OnResized(
			const Vec2ui& aClientSize,
			const Vec2ui& aWindowSize);

		void OnDropFiles(WPARAM aWparam);

		void SetWindowState(const WindowState aWindowState);
		void SetWindowResolution(const Vec2ui& aResolution);
		void SetVSync(bool aState);
		void DrawDebugMenu();

		const Vec2ui& GetPosition() const;

		void SetCursorMode(const CursorMode aCursorMode);
		CursorMode GetCursorMode() const;

	private:
		void TryUpdatingSize();
		void ShowCursorInternal(const bool aVisible);

	private:
		Directx11Framework* myFramework = nullptr;

		HWND myWindowHandle = nullptr;
		HICON myIcon = nullptr;

		WindowData myWindowData;
		WindowState myWindowState = WindowState::Unset;


		// ONLY used in resizing messages
		bool myIsInSizeMove = false;

		bool myIsMinimized = false;

		UINT myClientWidth = 0;
		UINT myClientHeight = 0;

		UINT myWindowWidth = 0;
		UINT myWindowHeight = 0;

		Vec2ui myPosition;

		bool myIsDestroyed = false;

		bool myIsInFocus = false;

		bool myIsCursorVisible = false;
		CursorMode myCursorMode = CursorMode::None;
		CursorMode myCursorModeBeforeLostFocus = CursorMode::None;
		bool myCursorVisibleBeforeLostFocus = true;

		// The internal state to avoid calling ShowCursor() more
		// than necessary because its ref counted internally in windows
		// bool myIsCursorVisibleInternal = false;
	};

	template <typename T>
	C::Vector2<T> WindowHandler::GetRenderingSize() const
	{
		return C::Vector2<T>(
			static_cast<T>(myWindowData.myRenderingSize.x),
			static_cast<T>(myWindowData.myRenderingSize.y));
	}

	template <typename T>
	C::Vector2<T> WindowHandler::GetWindowSize() const
	{
		return C::Vector2<T>(
			static_cast<T>(myWindowData.myWindowSize.x),
			static_cast<T>(myWindowData.myWindowSize.y));
	}
}