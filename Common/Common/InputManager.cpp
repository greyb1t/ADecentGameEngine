#include <assert.h>
#include <Windows.h>
#include <hidusage.h>

#include "InputManager.h"
#include "Common/Math/Math.h"
// #include "Common/Math/Math.h"

void Common::InputManager::SetWindow(HWND aWindowHandle)
{
	// Register the raw input to the window to be able to receive WM_INPUT
	assert(aWindowHandle);

	RAWINPUTDEVICE rid;
	rid.usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid.usUsage = HID_USAGE_GENERIC_MOUSE;
	rid.dwFlags = RIDEV_INPUTSINK;
	rid.hwndTarget = aWindowHandle;

	if (!RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE)))
	{
		assert(false);
	}
}

void Common::InputManager::BeginFrame()
{
	if (!myIsWindowActive)
	{
		// When window is deactivated, ensure we release all keys so none of them
		// become stuck
		// We simulate a release by setting the all current to false
		// that way, the previous key check will still go through and 
		// all "key release" checks will hit when alt-tabbing while holding
		// a key down
		myCurrentKeyStates = {};

		// Stop the camera from moving when not tabbed in
		myRawInputMouseDelta = {};
	}
}

#include <iostream>
void Common::InputManager::Update()
{
	myPrevShotState = myShotState;
	myShotState = IsInputShoot();

	myPrevSlowMoState = mySlowMoState;
	mySlowMoState = IsInputSlowMo();

	myPrevDashState = myDashState;
	myDashState = IsInputDash();

	myPrevHealState = myHealState;
	myHealState = IsInputHeal();


	myDeltaCursorPosition = {
		myCurrentCursorPosition.x - myPreviousCursorPosition.x + myMousePosWrapDelta.x,
		myCurrentCursorPosition.y - myPreviousCursorPosition.y + myMousePosWrapDelta.y
	};

	myPreviousCursorPositionAbsolute = myAbsoluteCursorPos;

	myPreviousCursorPosition = myCurrentCursorPosition;

	myPreviousKeyStates = myCurrentKeyStates;

	myMouseWheelDelta = 0;

	GetCursorPos(&myAbsoluteCursorPos);

	myDeltaCursorPositionAbsolute = {
		myAbsoluteCursorPos.x - myPreviousCursorPositionAbsolute.x + myMousePosWrapDelta.x,
		myAbsoluteCursorPos.y - myPreviousCursorPositionAbsolute.y + myMousePosWrapDelta.y,
	};

	myPreviousCursorPositionAbsolute = myAbsoluteCursorPos;


	myWrappedMousePos = false;
	myMousePosWrapDelta = { };
	myRawInputMouseDelta = { };
	//// TODO: Add a mouse mode, absolute or relative ?
	//if (myWrapMouseEnabled /* if relative */)
	//{
	//	POINT p;
	//	GetCursorPos(&p);
	//	SetCursorPos(400, 400);
	//
	//	myMousePosWrapDelta.x = p.x - 400;
	//	myMousePosWrapDelta.y = p.y - 400;
	//}

	myControllerInput.Refresh();
}

short Common::InputManager::GetMouseWheelDelta() const
{
	return myMouseWheelDelta;
}

int GetRightOrLeftKeyCode(const int aVirtualKeyCode, const LPARAM aLparam)
{
	int resultKeyCode = aVirtualKeyCode;

	const auto scanCode = static_cast<UINT>((aLparam & 0x00ff0000) >> 16);
	const bool isExtended = (aLparam & 0x01000000);

	switch (aVirtualKeyCode)
	{
	case VK_SHIFT:
		// Convert the scan code to a virtual key code
		resultKeyCode = MapVirtualKey(scanCode, MAPVK_VSC_TO_VK_EX);
		break;
	case VK_CONTROL:
		resultKeyCode = isExtended ? VK_RCONTROL : VK_LCONTROL;
		break;
	case VK_MENU:
		resultKeyCode = isExtended ? VK_RMENU : VK_LMENU;
		break;
	default:
		break;
	}

	return resultKeyCode;
}

bool Common::InputManager::UpdateEvents(HWND aWindowHandle, UINT aMessage, WPARAM aWparam, LPARAM aLparam)
{
	switch (aMessage)
	{
	case WM_ACTIVATE:
	{
		const bool isActivated =
			(LOWORD(aWparam) == WA_ACTIVE) ||
			(LOWORD(aWparam) == WA_CLICKACTIVE);

		if (isActivated)
		{
			myIsWindowActive = true;
		}
		else if (LOWORD(aWparam) == WA_INACTIVE)
		{
			myIsWindowActive = false;
		}
	} break;

	case WM_INPUT:
	{
		UINT dwSize = sizeof(RAWINPUT);
		static BYTE lpb[sizeof(RAWINPUT)];

		GetRawInputData(
			reinterpret_cast<HRAWINPUT>(aLparam),
			RID_INPUT,
			lpb,
			&dwSize,
			sizeof(RAWINPUTHEADER));

		const auto* raw = reinterpret_cast<RAWINPUT*>(lpb);

		if (raw->header.dwType == RIM_TYPEMOUSE)
		{
			int xPosRelative = raw->data.mouse.lLastX;
			int yPosRelative = raw->data.mouse.lLastY;

			myRawInputMouseDelta.x += xPosRelative;
			myRawInputMouseDelta.y += yPosRelative;
		}
	} break;

	case WM_MOUSEWHEEL:
	{
		myMouseWheelDelta = GET_WHEEL_DELTA_WPARAM(aWparam) / WHEEL_DELTA;
	} break;

	case WM_MOUSEMOVE:
	{
		if (myWrappedMousePos)
		{
			break;
		}

		POINTS points = MAKEPOINTS(aLparam);

		myCurrentCursorPosition = { points.x, points.y };

		// TODO: Only call this once when the window was resized?

		//if (myWrapMouseEnabled)
		//{
		//	RECT windowRect;
		//	GetWindowRect(aWindowHandle, &windowRect);

		//	RECT rect;
		//	GetClientRect(aWindowHandle, &rect);

		//	// Added to avoid the issue with the next if statement occuring immediately 
		//	// causing an unlimited flicking thingy dingy
		//	const int padding = 10;

		//	if (myCurrentCursorPosition.x >= (rect.right - 1))
		//	{
		//		POINT cursorPosInScreenSpace = myCurrentCursorPosition;
		//		ClientToScreen(aWindowHandle, &cursorPosInScreenSpace);

		//		// Convert the window client coordinates to screen coordinates
		//		RECT clientRectScreenSpace = rect;
		//		MapWindowPoints(aWindowHandle, NULL, reinterpret_cast<LPPOINT>(&clientRectScreenSpace), 2);

		//		SetCursorPos(
		//			clientRectScreenSpace.left + (myCurrentCursorPosition.x - rect.right - 1) + padding,
		//			cursorPosInScreenSpace.y);

		//		myMousePosWrapDelta.x = rect.right - rect.left - 1 - padding;

		//		myWrappedMousePos = true;
		//	}
		//	else if (myCurrentCursorPosition.x <= (rect.left + 1))
		//	{
		//		POINT cursorPosInScreenSpace = myCurrentCursorPosition;
		//		ClientToScreen(aWindowHandle, &cursorPosInScreenSpace);

		//		// Convert the window client coordinates to screen coordinates
		//		RECT clientRectScreenSpace = rect;
		//		MapWindowPoints(aWindowHandle, NULL, reinterpret_cast<LPPOINT>(&clientRectScreenSpace), 2);

		//		SetCursorPos(
		//			clientRectScreenSpace.right + (myCurrentCursorPosition.x - rect.left - 1) - padding,
		//			cursorPosInScreenSpace.y);

		//		myMousePosWrapDelta.x = rect.left - rect.right - 1 + padding;

		//		myWrappedMousePos = true;
		//	}

		//	if (myCurrentCursorPosition.y >= (rect.bottom - 1))
		//	{
		//		POINT cursorPosInScreenSpace = myCurrentCursorPosition;
		//		ClientToScreen(aWindowHandle, &cursorPosInScreenSpace);

		//		// Convert the window client coordinates to screen coordinates
		//		RECT clientRectScreenSpace = rect;
		//		MapWindowPoints(aWindowHandle, NULL, reinterpret_cast<LPPOINT>(&clientRectScreenSpace), 2);

		//		SetCursorPos(
		//			cursorPosInScreenSpace.x,
		//			clientRectScreenSpace.top + (myCurrentCursorPosition.y - rect.bottom - 1) + padding);

		//		myMousePosWrapDelta.y = rect.bottom - rect.top - 1 - padding;

		//		myWrappedMousePos = true;
		//	}
		//	else if (myCurrentCursorPosition.y <= (rect.top + 1))
		//	{
		//		POINT cursorPosInScreenSpace = myCurrentCursorPosition;
		//		ClientToScreen(aWindowHandle, &cursorPosInScreenSpace);

		//		// Convert the window client coordinates to screen coordinates
		//		RECT clientRectScreenSpace = rect;
		//		MapWindowPoints(aWindowHandle, NULL, reinterpret_cast<LPPOINT>(&clientRectScreenSpace), 2);

		//		SetCursorPos(
		//			cursorPosInScreenSpace.x,
		//			clientRectScreenSpace.bottom + (myCurrentCursorPosition.y - rect.top - 1) - padding);

		//		myMousePosWrapDelta.y = rect.top - rect.bottom - 1 + padding;

		//		myWrappedMousePos = true;
		//	}
		//}
	} break;

	case WM_XBUTTONUP:
	{
		switch (HIWORD(aWparam))
		{
		case XBUTTON1:
		{
			myCurrentKeyStates[static_cast<int>(MouseButton::XButton1)] = false;
		} break;
		case XBUTTON2:
		{
			myCurrentKeyStates[static_cast<int>(MouseButton::XButton2)] = false;
		} break;
		default:
			break;
		}
	} break;

	case WM_XBUTTONDOWN:
	{
		switch (HIWORD(aWparam))
		{
		case XBUTTON1:
		{
			myCurrentKeyStates[static_cast<int>(MouseButton::XButton1)] = true;
		} break;
		case XBUTTON2:
		{
			myCurrentKeyStates[static_cast<int>(MouseButton::XButton2)] = true;
		} break;
		default:
			break;
		}
	} break;

	case WM_MBUTTONUP:
	{
		myCurrentKeyStates[static_cast<int>(MouseButton::Middle)] = false;
	} break;

	case WM_MBUTTONDOWN:
	{
		myCurrentKeyStates[static_cast<int>(MouseButton::Middle)] = true;
	} break;

	case WM_RBUTTONUP:
	{
		myCurrentKeyStates[static_cast<int>(MouseButton::Right)] = false;
	} break;

	case WM_RBUTTONDOWN:
	{
		myCurrentKeyStates[static_cast<int>(MouseButton::Right)] = true;
	} break;

	case WM_LBUTTONUP:
	{
		myCurrentKeyStates[static_cast<int>(MouseButton::Left)] = false;
	} break;

	case WM_LBUTTONDOWN:
	{
		myCurrentKeyStates[static_cast<int>(MouseButton::Left)] = true;
	} break;

	case WM_KEYUP:
	{
		int keyCode = static_cast<int>(aWparam);

		switch (keyCode)
		{
		case VK_SHIFT:
		case VK_CONTROL:
		case VK_MENU:
			keyCode = GetRightOrLeftKeyCode(keyCode, aLparam);
			break;
		default:
			break;
		}

		myCurrentKeyStates[keyCode] = false;
	} break;

	case WM_KEYDOWN:
	{
		int keyCode = static_cast<int>(aWparam);

		switch (keyCode)
		{
		case VK_SHIFT:
		case VK_CONTROL:
		case VK_MENU:
			keyCode = GetRightOrLeftKeyCode(keyCode, aLparam);
			break;
		default:
			break;
		}

		myCurrentKeyStates[keyCode] = true;
	} break;

	default:
		// Message was not handled
		return false;
	}

	// Message was handled
	return true;
}

void Common::InputManager::SetMouseWrapping(const bool aWrapMouse)
{
	myWrapMouseEnabled = aWrapMouse;
}

void Common::InputManager::ShowCursorGuaranteed()
{
	return;

	myIsShowingCursor = true;

	while (true)
	{
		CURSORINFO ci = { };
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

void Common::InputManager::HideCursorGuaranteed()
{
	return;

	myIsShowingCursor = false;

	while (true)
	{
		CURSORINFO ci = { };
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

const bool& Common::InputManager::IsShowingCursor() const
{
	assert(false && "do not use");
	return myIsShowingCursor;
}

float Common::InputManager::GetLeftRightInput()
{
	if (IsKey(KeyCode::A))
		return -1.0f;

	if (IsKey(KeyCode::D))
		return 1.0f;

	return myControllerInput.leftStickX;
}

float Common::InputManager::GetForwardBackInput()
{
	if (IsKey(KeyCode::W))
		return 1.0f;

	if (IsKey(KeyCode::S))
		return -1.0f;

	return myControllerInput.leftStickY;
}

bool Common::InputManager::IsInputShoot()
{
	if (myControllerInput.rightTrigger > 0.1f)
		return true;

	if (IsMouseKeyPressed(MouseButton::Left) || IsMouseKeyDown(MouseButton::Left))
		return true;

	return false;
}

bool Common::InputManager::IsInputShootDown()
{
	return myShotState && !myPrevShotState;
}

bool Common::InputManager::IsInputAim()
{
	if (myControllerInput.leftTrigger > 0.1f)
		return true;

	if (IsMouseKeyPressed(MouseButton::Right))
		return true;

	return false;
}

bool Common::InputManager::IsInputDash()
{
	if (myControllerInput.IsPressed(XINPUT_GAMEPAD_RIGHT_SHOULDER))
		return true;

	if (IsKeyPressed(KeyCode::Space))
		return true;

	return false;
}

bool Common::InputManager::IsInputDashDown()
{
	return myDashState && !myPrevDashState;
}

bool Common::InputManager::IsInputSlowMo()
{
	if (myControllerInput.IsPressed(XINPUT_GAMEPAD_A))
		return true;

	if (IsKeyPressed(KeyCode::LeftShift))
		return true;

	return false;
}

bool Common::InputManager::IsInputSlowMoDown()
{
	return mySlowMoState && !myPrevSlowMoState;
}

bool Common::InputManager::IsInputHeal()
{
	if (myControllerInput.IsPressed(XINPUT_GAMEPAD_DPAD_UP))
		return true;

	if (IsKeyPressed(KeyCode::Tab))
		return true;

	return false;
}

bool Common::InputManager::IsInputHealDown()
{
	return myHealState && !myPrevHealState;
}


float Common::InputManager::GetRightX() const
{
	return myControllerInput.rightStickX;
}

float Common::InputManager::GetRightY() const
{
	return myControllerInput.rightStickY;
}

float Common::InputManager::GetLeftX() const
{
	return myControllerInput.leftStickX;
}

float Common::InputManager::GetLeftY() const
{
	return myControllerInput.leftStickY;
}

int Common::InputManager::GetWeaponCycle() const
{
	int scrollVal = GetMouseWheelDelta();

	if (scrollVal != 0)
		return scrollVal;


	static bool wasPressedLastFrame = false;
	if (myControllerInput.IsPressed(XINPUT_GAMEPAD_LEFT_SHOULDER) && !wasPressedLastFrame)
	{
		wasPressedLastFrame = true;
		return 1;
	}
	else if (!myControllerInput.IsPressed(XINPUT_GAMEPAD_LEFT_SHOULDER) && wasPressedLastFrame)
	{
		wasPressedLastFrame = false;
	}


	return 0;
}


bool Common::InputManager::IsKeyDown(const KeyCode aKeyCode) const
{
	return myCurrentKeyStates[static_cast<int>(aKeyCode)] &&
		!myPreviousKeyStates[static_cast<int>(aKeyCode)];
}

bool Common::InputManager::IsKeyPressed(const KeyCode aKeyCode) const
{
	return myCurrentKeyStates[static_cast<int>(aKeyCode)];
}

bool Common::InputManager::WasKeyReleased(const KeyCode aKeyCode) const
{
	return !myCurrentKeyStates[static_cast<int>(aKeyCode)] &&
		myPreviousKeyStates[static_cast<int>(aKeyCode)];
}

bool Common::InputManager::IsKey(const KeyCode aKeyCode) const
{
	return (IsKeyDown(aKeyCode) || IsKeyPressed(aKeyCode));
}


bool Common::InputManager::IsAnyKeyDown() const
{
	for (int i = 0; i < 256; ++i)
	{
		if (IsKeyDown(static_cast<CU::KeyCode>(i)))
		{
			return true;
		}
	}

	return false;
}

bool Common::InputManager::IsMouseKeyDown(const MouseButton aMouseButton) const
{
	return myCurrentKeyStates[static_cast<int>(aMouseButton)] &&
		!myPreviousKeyStates[static_cast<int>(aMouseButton)];
}

bool Common::InputManager::IsMouseKeyPressed(const MouseButton aMouseButton) const
{
	return myCurrentKeyStates[static_cast<int>(aMouseButton)];
}

bool Common::InputManager::WasMouseKeyReleased(const MouseButton aMouseButton) const
{
	return !myCurrentKeyStates[static_cast<int>(aMouseButton)] &&
		myPreviousKeyStates[static_cast<int>(aMouseButton)];
}

Common::Vector2ui Common::InputManager::GetMousePosition() const
{
	return Common::Vector2ui(
		static_cast<unsigned int>(myCurrentCursorPosition.x),
		static_cast<unsigned int>(myCurrentCursorPosition.y));
}

POINT Common::InputManager::GetMousePositionAbsolute() const
{
	return myAbsoluteCursorPos;
}

POINT Common::InputManager::GetMouseDelta() const
{
	return myDeltaCursorPosition;
}

POINT Common::InputManager::GetMouseDeltaRaw() const
{
	return myRawInputMouseDelta;
}

POINT Common::InputManager::GetMouseDeltaAbsolute() const
{
	return myDeltaCursorPositionAbsolute;
}

void Common::InputManager::SetMousePosition(const POINT& aPoint) const
{
	const BOOL ret = SetCursorPos(aPoint.x, aPoint.y);
	assert(ret && "SetCursorPos failed");
}

void Common::InputManager::LockCursorToWindow(const HWND aWindowHandle)
{
	GetClipCursor(&myOldClipRect);

	RECT clientRect;
	GetClientRect(aWindowHandle, &clientRect);

	// Convert the window client coordinates to screen coordinates
	MapWindowPoints(aWindowHandle, NULL, reinterpret_cast<LPPOINT>(&clientRect), 2);

	ClipCursor(&clientRect);
}

void Common::InputManager::UnlockCursorToWindow() const
{
	ClipCursor(nullptr);

	/*
	const RECT oldClipRect = myOldClipRect;
	if (oldClipRect.left == 0 && oldClipRect.bottom == 0 &&
		oldClipRect.right == 0 && oldClipRect.top == 0)
	{
		ClipCursor(nullptr);
	}
	else
	{
		ClipCursor(&oldClipRect);
	}
	*/
}
