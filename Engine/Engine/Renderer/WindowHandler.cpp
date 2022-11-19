#include "pch.h"
#include "WindowHandler.h"
#include "Directx11Framework.h"
#include "Engine/DebugManager\DebugMenu.h"
#include "Engine/Engine.h"
#include "Engine/Scene/SceneManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Editor/Editor.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Engine
{
	void ShowCursorGuaranteed()
	{
		while (true)
		{
			CURSORINFO ci = {};
			ci.cbSize = sizeof(CURSORINFO);
			GetCursorInfo(&ci);

			if (ci.flags == 0)
			{
				ShowCursor(TRUE);
			}
			else
			{
				break;
			}
		}
	}

	void HideCursorGuaranteed()
	{
		while (true)
		{
			CURSORINFO ci = {};
			ci.cbSize = sizeof(CURSORINFO);
			GetCursorInfo(&ci);

			if (ci.flags != 0)
			{
				ShowCursor(FALSE);
			}
			else
			{
				break;
			}
		}
	}

	WindowHandler::~WindowHandler()
	{
		if (myIcon)
		{
			DestroyIcon(myIcon);
			myIcon = nullptr;
		}
	}

	bool WindowHandler::Init(const WindowData aWindowData, Directx11Framework& aFramework)
	{
		myWindowData = aWindowData;
		myFramework = &aFramework;

		myWindowState = myWindowData.myWindowState;

		const wchar_t* className = L"Dx11WindowClass";

		// From Launcher resource.h
		const int iconId = 101;

		myIcon = reinterpret_cast<HICON>(
			LoadImage(
				aWindowData.myInstanceHandle,
				MAKEINTRESOURCE(iconId),
				IMAGE_ICON, 0, 0, 0));

		WNDCLASS windowClass = {};
		windowClass.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
		windowClass.lpfnWndProc = WindowHandler::WndProc;
		windowClass.hInstance = aWindowData.myInstanceHandle;
		windowClass.hIcon = myIcon;
		windowClass.hCursor = nullptr;
		windowClass.lpszClassName = className;
		windowClass.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));

		const ATOM atom = RegisterClass(&windowClass);

		if (!atom)
		{
			LOG_ERROR(LogType::Renderer) << "RegisterClass failed " << GetLastError();

			return false;
		}

		LONG_PTR style = WS_VISIBLE;

		if (aWindowData.myWindowState == WindowState::Windowed)
		{
			style |= WS_OVERLAPPEDWINDOW;
		}
		else if (aWindowData.myWindowState == WindowState::WindowedBorderless)
		{
			style |= WS_POPUP;
		}
		else
		{
			assert(false);
		}

		myWindowHandle = CreateWindow(
			className,
			aWindowData.myTitle.c_str(),
			style | WS_VISIBLE | WS_MAXIMIZE,
			0,
			0,
			aWindowData.myWindowSize.x,
			aWindowData.myWindowSize.y,
			nullptr,
			nullptr,
			aWindowData.myInstanceHandle,
			this);

		if (!GetWindowHandle())
		{
			LOG_ERROR(LogType::Renderer) << "CreateWindow failed " << GetLastError();

			return false;
		}

		if (aWindowData.myWindowFlags & WindowFlags::WindowFlags_CalculateRenderingSizeAutomatically)
		{
			RECT rect = {};
			GetClientRect(myWindowHandle, &rect);
			myWindowData.myRenderingSize = Vec2ui(rect.right - rect.left, rect.bottom - rect.top);
		}

		return true;
	}

	void WindowHandler::Update()
	{
		switch (myCursorMode)
		{
		case CursorMode::CenterLocked:
		{
			const auto& viewport = GetEngine().GetViewport();

			const int x = static_cast<int>(
				viewport.myPosition.x + (viewport.myRendererSize.x) / 2.f);
			const int y = static_cast<int>(
				viewport.myPosition.y + (viewport.myRendererSize.y) / 2.f);

			SetCursorPos(myPosition.x + x, myPosition.y + y);
		} break;
		case CursorMode::None:
			break;
		default:
			break;
		}
	}

	void WindowHandler::SetCursorVisibility(const bool aVisible)
	{
		if (!myIsInFocus)
		{
			// When giving window focus again, it will 
			// use the most recent cursor visiblity
			myCursorVisibleBeforeLostFocus = aVisible;
			return;
		}

		// If actually changed it, call the win32 function
		// must do this because internally its ref counted
		// if (myIsCursorVisibleInternal != aVisible)
		{
			ShowCursorInternal(static_cast<BOOL>(aVisible));
			// myIsCursorVisibleInternal = aVisible;
		}

		myIsCursorVisible = aVisible;
	}

	bool WindowHandler::IsCursorVisible() const
	{
		return myIsCursorVisible;
	}

	HCURSOR WindowHandler::GetCursorHandle() const
	{
		return myWindowData.myCursorHandle;
	}

	void WindowHandler::SetCursorHandle(HCURSOR aCursorHandle)
	{
		myWindowData.myCursorHandle = aCursorHandle;
	}

	LRESULT CALLBACK WindowHandler::WndProc(
		HWND aWindowHandle, UINT aMessage, WPARAM aWparam, LPARAM aLparam)
	{
		static WindowHandler* windowHandler = nullptr;

		switch (aMessage)
		{
		case WM_DROPFILES:
		{
			windowHandler->OnDropFiles(aWparam);
		} break;

		case WM_DESTROY:
		{
			windowHandler->SetIsDestroyed(true);
			PostQuitMessage(0);
			return 0;
		} break;

		case WM_CREATE:
		{
			const auto createStruct = reinterpret_cast<CREATESTRUCT*>(aLparam);
			windowHandler = reinterpret_cast<WindowHandler*>(createStruct->lpCreateParams);
			DragAcceptFiles(aWindowHandle, TRUE);
			return 0;
		} break;

		case WM_ENTERSIZEMOVE:
			windowHandler->myIsInSizeMove = true;
			break;

		case WM_EXITSIZEMOVE:
		{
			windowHandler->myIsInSizeMove = false;

			// RECT rect;
			// GetWindowRect(aWindowHandle, &rect);
			// windowHandler->myWindowWidth = rect.right - rect.left;
			// windowHandler->myWindowHeight = rect.bottom - rect.top;

			//windowHandler->TryUpdatingSize();
		} break;

		case WM_SIZE:
		{
			const UINT clientWidth = LOWORD(aLparam);
			const UINT clientHeight = HIWORD(aLparam);

			windowHandler->myClientWidth = clientWidth;
			windowHandler->myClientHeight = clientHeight;

			RECT rect;
			GetWindowRect(aWindowHandle, &rect);
			windowHandler->myWindowWidth = rect.right - rect.left;
			windowHandler->myWindowHeight = rect.bottom - rect.top;

			if (!windowHandler->myIsInSizeMove)
			{
				if (aWparam == SIZE_MINIMIZED)
				{
					// if we minimized, we do not want to change size of textures
					// because they would be 0 and fail
					windowHandler->myIsMinimized = true;
					LOG_INFO(LogType::Engine) << "Minimized window";
				}
				else if (aWparam == SIZE_RESTORED)
				{
					windowHandler->myIsMinimized = false;
					LOG_INFO(LogType::Engine) << "Restored window";

					windowHandler->TryUpdatingSize();
				}
				else
				{
					// If we did anything else, update the resolution
					windowHandler->TryUpdatingSize();
				}
			}
			else
			{
				// https://docs.microsoft.com/en-us/windows/win32/direct3darticles/dxgi-best-practices
				// All that the application needs to do is call IDXGISwapChain::ResizeBuffers to 
				// resize the back buffer to the size that was passed as parameters in WM_SIZE
				windowHandler->TryUpdatingSize();
			}
		} break;

		case WM_GETMINMAXINFO:
		{
			auto info = reinterpret_cast<MINMAXINFO*>(aLparam);

			info->ptMinTrackSize.x = 320;
			info->ptMinTrackSize.y = 200;
		}
		break;

		case WM_MOVE:
		{
			const auto xPos = (int)(short)LOWORD(aLparam);
			const auto yPos = (int)(short)HIWORD(aLparam);

			windowHandler->myPosition.x = xPos;
			windowHandler->myPosition.y = yPos;
		} break;

		case WM_SETFOCUS:
		{
			// Must be first
			windowHandler->myIsInFocus = true;

			// When window is focused
			// Returns to the previous cursor settings
			windowHandler->SetCursorMode(windowHandler->myCursorModeBeforeLostFocus);
			windowHandler->SetCursorVisibility(windowHandler->myCursorVisibleBeforeLostFocus);

			windowHandler->myCursorModeBeforeLostFocus = CursorMode::None;
			windowHandler->myCursorVisibleBeforeLostFocus = true;
		} break;
		case WM_KILLFOCUS:
		{
			// When losing focus, must give back cursor control
				// and make it visible
			windowHandler->myCursorVisibleBeforeLostFocus = windowHandler->myIsCursorVisible;
			windowHandler->myCursorModeBeforeLostFocus = windowHandler->myCursorMode;

			windowHandler->SetCursorMode(CursorMode::None);
			windowHandler->SetCursorVisibility(true);
			// windowHandler->SetHideCursor

			// Must be last
			windowHandler->myIsInFocus = false;
		} break;

		//case WM_ACTIVATE:
		//{
		//	const bool isActivated =
		//		(LOWORD(aWparam) == WA_ACTIVE) ||
		//		(LOWORD(aWparam) == WA_CLICKACTIVE);

		//	if (isActivated)
		//	{
		//		// Must be first
		//		windowHandler->myIsInFocus = true;

		//		// When window is focused
		//		// Returns to the previous cursor settings
		//		windowHandler->SetCursorMode(windowHandler->myCursorModeBeforeLostFocus);
		//		windowHandler->SetCursorVisibility(windowHandler->myCursorVisibleBeforeLostFocus);

		//		windowHandler->myCursorModeBeforeLostFocus = CursorMode::None;
		//		windowHandler->myCursorVisibleBeforeLostFocus = true;
		//	}
		//	else if (LOWORD(aWparam) == WA_INACTIVE)
		//	{
		//		// When losing focus, must give back cursor control
		//		// and make it visible
		//		windowHandler->myCursorVisibleBeforeLostFocus = windowHandler->myIsCursorVisible;
		//		windowHandler->myCursorModeBeforeLostFocus = windowHandler->myCursorMode;

		//		windowHandler->SetCursorMode(CursorMode::None);
		//		windowHandler->SetCursorVisibility(true);
		//		// windowHandler->SetHideCursor

		//		// Must be last
		//		windowHandler->myIsInFocus = false;
		//	}
		//	else
		//	{
		//		assert(false);
		//	}
		//} break;

		case WM_SETCURSOR:
		{
			// Custom cursor
			if (windowHandler->IsCursorVisible() && LOWORD(aLparam) == HTCLIENT)
			{
				SetCursor(windowHandler->GetCursorHandle());

				// ShowCursorGuaranteed();
				// For some reason, in some cases the cursor
				// disappears when alt-tabbing out of the game.
				// To make it appear again, simply check if its hidden
				// here and show it once
				/*
				CURSORINFO ci = {};
				ci.cbSize = sizeof(CURSORINFO);
				GetCursorInfo(&ci);

				bool isHidden = ci.flags == 0;

				if (isHidden)
				{
					ShowCursor(TRUE);
				}
				*/

				return 1;
			}

		} break;

		default:
			break;
		}

		// ORDER IS IMPORTANT HERE
		// WE NEED TO CALL THIS AFTER WE HANDLE STUFF ABOVE
		// E.G. IMGUI TRAPS WM_SETCURSOR AND WE CANNOT HANDLE IT OURSELVES...
		if (ImGui_ImplWin32_WndProcHandler(aWindowHandle, aMessage, aWparam, aLparam))
			return true;

		// Block mouse left click going through imgui
		if (aMessage == WM_LBUTTONDOWN)
		{
			if (ImGui::GetCurrentContext())
			{
				if (ImGui::GetIO().WantCaptureMouse)
				{
					return true;
				}
			}
		}

		// NOTE(filip): I remove this because when the Graph Node Editor was visible, it captures all
		// input, meaning that we could not move the camera in the in-game-editor
		//// Block the keyboard and mouse going through imgui
		//if (aMessage == WM_KEYDOWN)
		//{
		//	if (ImGui::GetCurrentContext())
		//	{
		//		if (ImGui::GetIO().WantCaptureKeyboard)
		//		{
		//			return true;
		//		}
		//	}
		//}

		if (windowHandler != nullptr && windowHandler->myWindowData.myWindowProc != nullptr)
		{
			if (windowHandler->myWindowData.myWindowProc(aWindowHandle, aMessage, aWparam, aLparam))
			{
				// TODO: handle return value
			}
		}

		return DefWindowProc(aWindowHandle, aMessage, aWparam, aLparam);
	}

	void WindowHandler::SetIsDestroyed(const bool aIsDestroyed)
	{
		myIsDestroyed = aIsDestroyed;
	}

	bool WindowHandler::IsDestroyed() const
	{
		return myIsDestroyed;
	}

	HWND WindowHandler::GetWindowHandle() const
	{
		return myWindowHandle;
	}

	const Vec2ui& WindowHandler::GetTargetSize() const
	{
		return myWindowData.myTargetSize;
	}

	void WindowHandler::OnResized(const Vec2ui& aClientSize, const Vec2ui& aWindowSize)
	{
		LOG_INFO(LogType::Engine) << "ClientSize: " << aClientSize.x << ", " << aClientSize.y;
		LOG_INFO(LogType::Engine) << "WindowSize: " << aWindowSize.x << ", " << aWindowSize.y;

		myWindowData.myRenderingSize = aClientSize;
		myWindowData.myWindowSize = aWindowSize;

		WindowResizedData resizeData;
		resizeData.myClientSize = aClientSize.CastTo<float>();
		resizeData.myWindowSize = aWindowSize.CastTo<float>();

		NotifyObservers(EventType::WindowResized, resizeData);
	}

	void WindowHandler::OnDropFiles(WPARAM aWparam)
	{
		const int pathBufferSize = 1024;

		std::vector<std::string> paths;

		const auto dropHandle = reinterpret_cast<HDROP>(aWparam);

		const auto pathsCount = DragQueryFileA(dropHandle, 0xFFFFFFFF, NULL, pathBufferSize);

		for (UINT i = 0; i < pathsCount; ++i)
		{
			const UINT filename_len = DragQueryFileA(dropHandle, i, nullptr, pathBufferSize) + 1;

			char buffer[pathBufferSize] = {};

			DragQueryFileA(dropHandle, i, buffer, filename_len);

			paths.push_back(buffer);
		}

		DragFinish(dropHandle);

		NotifyObservers(EventType::DraggedFileIntoWindow, paths);
	}

	void WindowHandler::SetWindowState(const WindowState aWindowState)
	{
		if (myWindowState == aWindowState)
		{
			return;
		}

		switch (aWindowState)
		{
		case WindowState::Unset:
			assert(false);
			break;
		case WindowState::Windowed:
		{
			auto currentStyle = GetWindowLongPtr(myWindowHandle, GWL_STYLE);
			currentStyle &= ~WS_POPUP;
			currentStyle |= WS_OVERLAPPEDWINDOW;
			SetWindowLongPtr(myWindowHandle, GWL_STYLE, currentStyle);
			SetWindowPos(myWindowHandle, HWND_TOP, 0, 0, 0, 0,
				SWP_FRAMECHANGED | SWP_NOREPOSITION | SWP_NOSIZE | SWP_NOMOVE);

			myFramework->GetSwapchain().SetFullscreenState(FALSE, nullptr);
		} break;

		case WindowState::WindowedBorderless:
		{
			auto currentStyle = GetWindowLongPtr(myWindowHandle, GWL_STYLE);
			currentStyle &= ~WS_OVERLAPPEDWINDOW;
			currentStyle |= WS_POPUP;
			SetWindowLongPtr(myWindowHandle, GWL_STYLE, currentStyle);
			SetWindowPos(myWindowHandle, HWND_TOP, 0, 0, 0, 0,
				SWP_FRAMECHANGED | SWP_NOREPOSITION | SWP_NOSIZE);

			myFramework->GetSwapchain().SetFullscreenState(FALSE, nullptr);
		} break;

		// best practices:
		// https://github.com/MicrosoftDocs/win32/blob/docs/desktop-src/direct3darticles/dxgi-best-practices.md

		case WindowState::Fullscreen:
		{
			const auto currentSize = Vec2ui(myWindowWidth, myWindowHeight);
			myFramework->GetSwapchain().SetFullscreenState(TRUE, nullptr);
			SetWindowResolution(currentSize);
		} break;
		default:
			break;
		}

		myWindowState = aWindowState;
	}

	void WindowHandler::SetWindowResolution(const Vec2ui& aResolution)
	{
		// SetWindowPos(myWindowHandle, HWND_TOP, 0, 0, aResolution.x, aResolution.y, SWP_FRAMECHANGED);
		DXGI_MODE_DESC desc = {};
		{
			desc.Width = aResolution.x;
			desc.Height = aResolution.y;
			// DXGI can automatically calculate this value if the application 
			// zeroes out the RefreshRate member of DXGI_MODE_DESC that is passed into ResizeTarget
			desc.RefreshRate = {};
			desc.Format = DXGI_FORMAT_UNKNOWN;
		}

		// Call after SetFullscreenState()
		myFramework->GetSwapchain().ResizeTarget(&desc);
	}

	void WindowHandler::SetVSync(bool aState)
	{
		myFramework->SetVSync(aState);
	}

	void WindowHandler::DrawDebugMenu()
	{
		DebugMenu::AddMenuEntry("Window", [this]()
			{
				if (ImGui::Button("Windowed Borderless"))
				{
					SetWindowState(WindowState::WindowedBorderless);
				}

				if (ImGui::Button("Windowed"))
				{
					SetWindowState(WindowState::Windowed);
				}

				if (ImGui::Button("Fullscreen"))
				{
					SetWindowState(WindowState::Fullscreen);
				}

				if (ImGui::Button("1920x1080"))
				{
					SetWindowResolution({ 1920, 1080 });
				}

				if (ImGui::Button("1280x720"))
				{
					SetWindowResolution({ 1280, 720 });
				}
			});
	}

	const Vec2ui& WindowHandler::GetPosition() const
	{
		return myPosition;
	}

	void WindowHandler::SetCursorMode(const CursorMode aCursorMode)
	{
		if (!myIsInFocus)
		{
			// When giving window focus again, it will 
			// use the most recent cursor mode
			myCursorModeBeforeLostFocus = aCursorMode;
			return;
		}

		myCursorMode = aCursorMode;
	}

	CursorMode WindowHandler::GetCursorMode() const
	{
		return myCursorMode;
	}

	void WindowHandler::TryUpdatingSize()
	{
		// If it changed
		if (myWindowData.myWindowSize.x == myWindowWidth &&
			myWindowData.myWindowSize.y == myWindowHeight &&
			myWindowData.myRenderingSize.x == myClientWidth &&
			myWindowData.myRenderingSize.y == myClientHeight)
		{
			return;
		}

		const auto clientSize = Vec2ui(
			myClientWidth,
			myClientHeight);

		const auto windowSize = Vec2ui(
			myWindowWidth,
			myWindowHeight);

		OnResized(clientSize, windowSize);
	}

	void WindowHandler::ShowCursorInternal(const bool aVisible)
	{
		CURSORINFO ci = {};
		ci.cbSize = sizeof(CURSORINFO);
		GetCursorInfo(&ci);

		const bool cursorVisible = (ci.flags & CURSOR_SHOWING) != 0;

		if (cursorVisible != aVisible)
		{
			if (aVisible)
			{
				ShowCursorGuaranteed();
			}
			else
			{
				HideCursorGuaranteed();
			}
			//ShowCursor(aVisible);
		}
	}
}