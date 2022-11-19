#pragma once

#include <array>
#include <Windows.h>
#include "Math/Math.h"

#include "Common/XInput.h"

namespace Common
{
	enum class KeyCode : unsigned char
	{
		Back = 0x8,
		Tab = 0x9,
		Enter = 0xd,
		Pause = 0x13,
		CapsLock = 0x14,
		Escape = 0x1b,
		Space = 0x20,
		PageUp = 0x21,
		PageDown = 0x22,
		End = 0x23,
		Home = 0x24,
		Left = 0x25,
		Up = 0x26,
		Right = 0x27,
		Down = 0x28,
		Select = 0x29,
		Print = 0x2a,
		Execute = 0x2b,
		PrintScreen = 0x2c,
		Insert = 0x2d,
		Delete = 0x2e,
		Help = 0x2f,

		D0 = 0x30,
		D1 = 0x31,
		D2 = 0x32,
		D3 = 0x33,
		D4 = 0x34,
		D5 = 0x35,
		D6 = 0x36,
		D7 = 0x37,
		D8 = 0x38,
		D9 = 0x39,

		A = 0x41,
		B = 0x42,
		C = 0x43,
		D = 0x44,
		E = 0x45,
		F = 0x46,
		G = 0x47,
		H = 0x48,
		I = 0x49,
		J = 0x4a,
		K = 0x4b,
		L = 0x4c,
		M = 0x4d,
		N = 0x4e,
		O = 0x4f,
		P = 0x50,
		Q = 0x51,
		R = 0x52,
		S = 0x53,
		T = 0x54,
		U = 0x55,
		V = 0x56,
		W = 0x57,
		X = 0x58,
		Y = 0x59,
		Z = 0x5a,

		LeftWindows = 0x5b,
		RightWindows = 0x5c,
		Apps = 0x5d,

		Sleep = 0x5f,
		NumPad0 = 0x60,
		NumPad1 = 0x61,
		NumPad2 = 0x62,
		NumPad3 = 0x63,
		NumPad4 = 0x64,
		NumPad5 = 0x65,
		NumPad6 = 0x66,
		NumPad7 = 0x67,
		NumPad8 = 0x68,
		NumPad9 = 0x69,
		Multiply = 0x6a,
		Add = 0x6b,
		Separator = 0x6c,
		Subtract = 0x6d,

		Decimal = 0x6e,
		Divide = 0x6f,
		F1 = 0x70,
		F2 = 0x71,
		F3 = 0x72,
		F4 = 0x73,
		F5 = 0x74,
		F6 = 0x75,
		F7 = 0x76,
		F8 = 0x77,
		F9 = 0x78,
		F10 = 0x79,
		F11 = 0x7a,
		F12 = 0x7b,
		F13 = 0x7c,
		F14 = 0x7d,
		F15 = 0x7e,
		F16 = 0x7f,
		F17 = 0x80,
		F18 = 0x81,
		F19 = 0x82,
		F20 = 0x83,
		F21 = 0x84,
		F22 = 0x85,
		F23 = 0x86,
		F24 = 0x87,

		NumLock = 0x90,
		Scroll = 0x91,

		LeftShift = 0xa0,
		RightShift = 0xa1,
		LeftControl = 0xa2,
		RightControl = 0xa3,
		LeftAlt = 0xa4,
		RightAlt = 0xa5,
		BrowserBack = 0xa6,
		BrowserForward = 0xa7,
		BrowserRefresh = 0xa8,
		BrowserStop = 0xa9,
		BrowserSearch = 0xaa,
		BrowserFavorites = 0xab,
		BrowserHome = 0xac,
		VolumeMute = 0xad,
		VolumeDown = 0xae,
		VolumeUp = 0xaf,
		MediaNextTrack = 0xb0,
		MediaPreviousTrack = 0xb1,
		MediaStop = 0xb2,
		MediaPlayPause = 0xb3,
		LaunchMail = 0xb4,
		SelectMedia = 0xb5,
		LaunchApplication1 = 0xb6,
		LaunchApplication2 = 0xb7,

		// The csgo console key
		Oem5 = 0xdc,

		Play = 0xfa,
		Zoom = 0xfb,
	};

	enum class MouseButton : unsigned char
	{
		Left = 0x01,
		Right = 0x02,
		Middle = 0x04,
		XButton1 = 0x05,
		XButton2 = 0x06,
	};

	class InputManager
	{
	public:
		InputManager() = default;

		void SetWindow(HWND aWindowHandle);

		// Call this at start of all updates
		void BeginFrame();

		// Call this at the end of all updates
		void Update();

		// Checks if a key was JUST pressed now
		bool IsKeyDown(const KeyCode aKeyCode) const;

		// Checks if a key is held down
		bool IsKeyPressed(const KeyCode aKeyCode) const;

		// Checks if a key was JUST released
		bool WasKeyReleased(const KeyCode aKeyCode) const;

		// Checks if a key is held or pressed down 
		bool IsKey(const KeyCode aKeyCode) const;

		bool IsAnyKeyDown() const;

		// Checks if mouse key was JUST pressed down
		bool IsMouseKeyDown(const MouseButton aMouseButton) const;

		// Checks is mouse key is held down
		bool IsMouseKeyPressed(const MouseButton aMouseButton) const;

		// Checks if mouse key was JUST released
		bool WasMouseKeyReleased(const MouseButton aMouseButton) const;

		Common::Vector2ui GetMousePosition() const;
		POINT GetMousePositionAbsolute() const;
		POINT GetMouseDelta() const;
		POINT GetMouseDeltaRaw() const;
		// Added to support getting a delta outside of the window
		POINT GetMouseDeltaAbsolute() const;

		void SetMousePosition(const POINT& aPoint) const;

		void LockCursorToWindow(const HWND aWindowHandle);
		void UnlockCursorToWindow() const;

		short GetMouseWheelDelta() const;

		bool UpdateEvents(HWND aWindowHandle, UINT aMessage, WPARAM aWparam, LPARAM aLparam);

		// Only works good when the cursor is locked inside the window
		void SetMouseWrapping(const bool aWrapMouse);

		void ShowCursorGuaranteed();
		void HideCursorGuaranteed();

		const bool& IsShowingCursor() const;

		//Gameplay input
		float GetLeftRightInput();
		float GetForwardBackInput();

		bool IsInputShoot();
		bool IsInputShootDown();

		bool IsInputAim();

		bool IsInputDash();
		bool IsInputDashDown();

		bool IsInputSlowMo();
		bool IsInputSlowMoDown();

		bool IsInputHeal();
		bool IsInputHealDown();

		//Controller Support
		float GetRightX() const;
		float GetRightY() const;

		float GetLeftX() const;
		float GetLeftY() const;

		//Returns -1 - 1. -1 for previous weapon, 1 for next, 0 is default
		int GetWeaponCycle() const;

	private:
		std::array<bool, 256> myCurrentKeyStates = {};
		std::array<bool, 256> myPreviousKeyStates = {};

		POINT myCurrentCursorPosition = {};
		POINT myPreviousCursorPosition = {};
		POINT myPreviousCursorPositionAbsolute = {};
		POINT myDeltaCursorPosition = {};
		POINT myDeltaCursorPositionAbsolute = {};

		POINT myRawInputMouseDelta = {};

		POINT myAbsoluteCursorPos = {};

		short myMouseWheelDelta = 0;

		RECT myOldClipRect = {};

		bool myWrappedMousePos = false;
		bool myWrapMouseEnabled = false;
		POINT myMousePosWrapDelta = {};

		bool myIsWindowActive = true;

		// MouseMode myMouseMode = MouseMode::Absolute;

		CXInput myControllerInput = CXInput(0.35f, 0.35f);

		bool myShotState = false;
		bool myPrevShotState = false;

		bool mySlowMoState = false;
		bool myPrevSlowMoState = false;

		bool myDashState = false;
		bool myPrevDashState = false;		
		
		bool myHealState = false;
		bool myPrevHealState = false;

		bool myIsShowingCursor = true;
	};
}

namespace CU = Common;