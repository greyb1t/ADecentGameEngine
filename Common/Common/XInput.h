#pragma once
#include <windows.h>
#include <Xinput.h>

// Buttons
/*
#define XINPUT_GAMEPAD_DPAD_UP          0x0001
#define XINPUT_GAMEPAD_DPAD_DOWN        0x0002
#define XINPUT_GAMEPAD_DPAD_LEFT        0x0004
#define XINPUT_GAMEPAD_DPAD_RIGHT       0x0008
#define XINPUT_GAMEPAD_START            0x0010
#define XINPUT_GAMEPAD_BACK             0x0020
#define XINPUT_GAMEPAD_LEFT_THUMB       0x0040
#define XINPUT_GAMEPAD_RIGHT_THUMB      0x0080
#define XINPUT_GAMEPAD_LEFT_SHOULDER    0x0100
#define XINPUT_GAMEPAD_RIGHT_SHOULDER   0x0200
#define XINPUT_GAMEPAD_A                0x1000
#define XINPUT_GAMEPAD_B                0x2000
#define XINPUT_GAMEPAD_X                0x4000
#define XINPUT_GAMEPAD_Y                0x8000
*/

namespace Common
{
	class CXInput
	{
	public:
		CXInput() : deadzoneX(0.05f), deadzoneY(0.02f) {}
		CXInput(float dzX, float dzY) : deadzoneX(dzX), deadzoneY(dzY) {}

		float leftStickX = 0.0f;
		float leftStickY = 0.0f;
		float rightStickX = 0.0f;
		float rightStickY = 0.0f;
		float leftTrigger = 0.0f;
		float rightTrigger = 0.0f;

		int  GetPort();
		XINPUT_GAMEPAD* GetState();
		bool CheckConnection();
		bool IsConnected(int aIndex);
		bool Refresh();
		bool IsPressed(WORD) const;

	private:
		int cId = -1;
		XINPUT_STATE state;
		float deadzoneX = 0.0f;
		float deadzoneY = 0.0f;
	};
}

