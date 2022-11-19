#include "XInput.h"

#include <math.h>
#pragma comment(lib, "Xinput9_1_0.lib")

using namespace std;
using namespace Common;

int CXInput::GetPort()
{
	return cId + 1;
}

XINPUT_GAMEPAD* CXInput::GetState()
{
	return &state.Gamepad;
}

bool CXInput::IsConnected(int aIndex)
{
	XINPUT_STATE state;
	ZeroMemory(&state, sizeof(XINPUT_STATE));

	if (XInputGetState(aIndex, &state) == ERROR_SUCCESS)
	{
		return true;
	}
	return false;
}

bool CXInput::CheckConnection()
{
	int controllerId = -1;

	for (DWORD i = 0; i < XUSER_MAX_COUNT && controllerId == -1; i++)
	{
		XINPUT_STATE state;
		ZeroMemory(&state, sizeof(XINPUT_STATE));

		if (XInputGetState(i, &state) == ERROR_SUCCESS)
			controllerId = i;
	}

	cId = controllerId;

	return controllerId != -1;
}

// Returns false if the controller has been disconnected
bool CXInput::Refresh()
{
	if (cId == -1)
		CheckConnection();

	if (cId != -1)
	{
		ZeroMemory(&state, sizeof(XINPUT_STATE));
		if (XInputGetState(cId, &state) != ERROR_SUCCESS)
		{
			cId = -1;
			return false;
		}

		float normLX = fmaxf(-1, (float)state.Gamepad.sThumbLX / 32767);
		float normLY = fmaxf(-1, (float)state.Gamepad.sThumbLY / 32767);

		leftStickX = (fabs(normLX) < deadzoneX ? 0 : (fabs(normLX) - deadzoneX) * (normLX / fabs(normLX)));
		leftStickY = (fabs(normLY) < deadzoneY ? 0 : (fabs(normLY) - deadzoneY) * (normLY / fabs(normLY)));

		if (deadzoneX > 0) leftStickX *= 1 / (1 - deadzoneX);
		if (deadzoneY > 0) leftStickY *= 1 / (1 - deadzoneY);

		float normRX = fmaxf(-1, (float)state.Gamepad.sThumbRX / 32767);
		float normRY = fmaxf(-1, (float)state.Gamepad.sThumbRY / 32767);

		rightStickX = (fabs(normRX) < deadzoneX ? 0 : (fabs(normRX) - deadzoneX) * (normRX / fabs(normRX)));
		rightStickY = (fabs(normRY) < deadzoneY ? 0 : (fabs(normRY) - deadzoneY) * (normRY / fabs(normRY)));

		if (deadzoneX > 0) rightStickX *= 1 / (1 - deadzoneX);
		if (deadzoneY > 0) rightStickY *= 1 / (1 - deadzoneY);

		leftTrigger = (float)state.Gamepad.bLeftTrigger / 255;
		rightTrigger = (float)state.Gamepad.bRightTrigger / 255;

		return true;
	}
	return false;
}

bool CXInput::IsPressed(WORD button) const
{
	return (state.Gamepad.wButtons & button) != 0;
}