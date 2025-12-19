#include "inputManager.h"
#include "audioPlayer.h"
#include "stringUtility.h"
#include "irRemote.h"

namespace
{
	bool mInitialized = false;
	HANDLE mControllerHandles[XGetPortCount()];
    HANDLE mRemoteHandles[XGetPortCount()];
    BYTE mRemoteStatesPrevious[XGetPortCount()];
    BYTE mRemoteStatesCurrent[XGetPortCount()];
	JoystickButtonStates mControllerStatesPrevious[XGetPortCount()];
	JoystickButtonStates mControllerStatesCurrent[XGetPortCount()];
	BYTE mTriggerLeftPressure[XGetPortCount()];
    BYTE mTriggerRightPressure[XGetPortCount()];
}

static bool sInOptionMode = false;

void inputManager::setSceneContext(bool inOptionMode)
{
	sInOptionMode = inOptionMode;
}

void inputManager::processController()
{
	if (mInitialized == false)
	{
		mInitialized = true;
		XInitDevices(0, 0);
		for (int i = 0; i < XGetPortCount(); i++)
		{
			mControllerHandles[i] = 0;
            memset(&mRemoteStatesPrevious[i], 0, sizeof(BYTE));
			memset(&mRemoteStatesCurrent[i], 0, sizeof(BYTE));
			memset(&mControllerStatesPrevious[i], 0, sizeof(JoystickButtonStates));
			memset(&mControllerStatesCurrent[i], 0, sizeof(JoystickButtonStates));
			mTriggerLeftPressure[i] = 0;
			mTriggerRightPressure[i] = 0;
		}
	}

    DWORD insertions = 0;
	DWORD removals = 0;
    if (XGetDeviceChanges(XDEVICE_TYPE_GAMEPAD, &insertions, &removals) == TRUE)
	{
		for (int i = 0; i < XGetPortCount(); i++)
		{
			if ((insertions & 1) == 1)
			{
				mControllerHandles[i] = XInputOpen(XDEVICE_TYPE_GAMEPAD, i, XDEVICE_NO_SLOT, NULL);
			}
			if ((removals & 1) == 1)
			{
				XInputClose(mControllerHandles[i]);
				mControllerHandles[i] = NULL;
			}
			insertions = insertions >> 1;
			removals = removals >> 1;
		}
	}

    if (XGetDeviceChanges(XDEVICE_TYPE_IR_REMOTE, &insertions, &removals) == TRUE)
	{
		for (int i = 0; i < XGetPortCount(); i++)
		{
			if ((insertions & 1) == 1)
			{
				mRemoteHandles[i] = XInputOpen(XDEVICE_TYPE_IR_REMOTE, i, XDEVICE_NO_SLOT, NULL);
			}
			if ((removals & 1) == 1)
			{
				XInputClose(mRemoteHandles[i]);
				mRemoteHandles[i] = NULL;
			}
			insertions = insertions >> 1;
			removals = removals >> 1;
		}
	}

    for (int i = 0; i < XGetPortCount(); i++)
	{
        memcpy(&mRemoteStatesPrevious[i], &mRemoteStatesCurrent[i], sizeof(BYTE));
		memset(&mRemoteStatesCurrent[i], 0, sizeof(BYTE));

        if (mRemoteHandles[i] == NULL)
		{
			continue;
		}

        XINPUT_STATEEX inputStates;
        memset(&inputStates, 0, sizeof(XINPUT_STATEEX));
		if (XInputGetState(mRemoteHandles[i], (XINPUT_STATE*)&inputStates) != 0) 
		{
			continue;
		}

        if (inputStates.firstEvent == 0x00)
        {
            continue;
        }

        mRemoteStatesCurrent[i] = inputStates.wButtons;
    }

	for (int i = 0; i < XGetPortCount(); i++)
	{
		memcpy(&mControllerStatesPrevious[i], &mControllerStatesCurrent[i], sizeof(JoystickButtonStates));
		memset(&mControllerStatesCurrent[i], 0, sizeof(JoystickButtonStates));
		mTriggerLeftPressure[i]  = 0;
		mTriggerRightPressure[i] = 0;

		if (mControllerHandles[i] == NULL)
		{
			continue;
		}

		XINPUT_STATE inputStates;
		if (XInputGetState(mControllerHandles[i], &inputStates) != 0) 
		{
			continue;
		}

		XINPUT_GAMEPAD gamePad;
		memcpy(&gamePad, &inputStates.Gamepad, sizeof(XINPUT_GAMEPAD));
		mTriggerLeftPressure[i]  = gamePad.bAnalogButtons[XINPUT_GAMEPAD_LEFT_TRIGGER];
		mTriggerRightPressure[i] = gamePad.bAnalogButtons[XINPUT_GAMEPAD_RIGHT_TRIGGER];

		if (gamePad.bAnalogButtons[XINPUT_GAMEPAD_A] > 32)
		{
			mControllerStatesCurrent[i].buttonA = ButtonStatePressed;
		}
		if (gamePad.bAnalogButtons[XINPUT_GAMEPAD_B] > 32)
		{
			mControllerStatesCurrent[i].buttonB = ButtonStatePressed;
		}
		if (gamePad.bAnalogButtons[XINPUT_GAMEPAD_X] > 32)
		{
			mControllerStatesCurrent[i].buttonX = ButtonStatePressed;
		}
		if (gamePad.bAnalogButtons[XINPUT_GAMEPAD_Y] > 32)
		{
			mControllerStatesCurrent[i].buttonY = ButtonStatePressed;
		}
		if (gamePad.bAnalogButtons[XINPUT_GAMEPAD_WHITE] > 32)
		{
			mControllerStatesCurrent[i].buttonWhite = ButtonStatePressed;
		}
		if (gamePad.bAnalogButtons[XINPUT_GAMEPAD_BLACK] > 32)
		{
			mControllerStatesCurrent[i].buttonBlack = ButtonStatePressed;
		}
		if ((gamePad.wButtons & XINPUT_GAMEPAD_BACK) != 0)
		{
			mControllerStatesCurrent[i].buttonBack = ButtonStatePressed;
		}
		if ((gamePad.wButtons & XINPUT_GAMEPAD_START) != 0)
		{
			mControllerStatesCurrent[i].buttonStart = ButtonStatePressed;
		}
		if ((gamePad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) != 0)
		{
			mControllerStatesCurrent[i].buttonLeftThumb = ButtonStatePressed;
		}
		if ((gamePad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) != 0)
		{
			mControllerStatesCurrent[i].buttonRightThumb = ButtonStatePressed;
		}
		if ((gamePad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0)
		{
			mControllerStatesCurrent[i].buttonDpadUp = ButtonStatePressed;
		}
		if ((gamePad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0)
		{
			mControllerStatesCurrent[i].buttonDpadRight = ButtonStatePressed;
		}
		if ((gamePad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0)
		{
			mControllerStatesCurrent[i].buttonDpadDown = ButtonStatePressed;
		}
		if ((gamePad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0)
		{
			mControllerStatesCurrent[i].buttonDpadLeft = ButtonStatePressed;
		}
		if (gamePad.bAnalogButtons[XINPUT_GAMEPAD_LEFT_TRIGGER] > 128)
		{
			mControllerStatesCurrent[i].buttonLeftTrigger = ButtonStatePressed;
		}
		if (gamePad.bAnalogButtons[XINPUT_GAMEPAD_RIGHT_TRIGGER] > 128)
		{
			mControllerStatesCurrent[i].buttonRightTrigger = ButtonStatePressed;
		}
	}
}

bool inputManager::buttonDown(JoystickButton button, int port)
{
	for (int i = 0; i < XGetPortCount(); i++)
	{
		if (port >= 0 && port != i)
		{
			continue;
		}

		if (mControllerHandles[i] == NULL)
		{
			continue;
		}

		if (button == JoystickButtonA && mControllerStatesCurrent[i].buttonA == ButtonStatePressed)
		{
			return true;
		}
		else if (button == JoystickButtonB && mControllerStatesCurrent[i].buttonB == ButtonStatePressed)
		{
			return true;
		}
		else if (button == JoystickButtonX && mControllerStatesCurrent[i].buttonX == ButtonStatePressed)
		{
			return true;
		}
		else if (button == JoystickButtonY && mControllerStatesCurrent[i].buttonY == ButtonStatePressed)
		{
			return true;
		}
		else if (button == JoystickButtonWhite && mControllerStatesCurrent[i].buttonWhite == ButtonStatePressed)
		{
			return true;
		}
		else if (button == JoystickButtonBlack && mControllerStatesCurrent[i].buttonBlack == ButtonStatePressed)
		{
			return true;
		}
		else if (button == JoystickButtonBack && mControllerStatesCurrent[i].buttonBack == ButtonStatePressed)
		{
			return true;
		}
		else if (button == JoystickButtonStart && mControllerStatesCurrent[i].buttonStart == ButtonStatePressed)
		{
			return true;
		}
		else if (button == JoystickButtonLeftThumb && mControllerStatesCurrent[i].buttonLeftThumb == ButtonStatePressed)
		{
			return true;
		}
		else if (button == JoystickButtonRightThumb && mControllerStatesCurrent[i].buttonRightThumb == ButtonStatePressed)
		{
			return true;
		}
		else if (button == JoystickButtonDpadUp && mControllerStatesCurrent[i].buttonDpadUp == ButtonStatePressed)
		{
			return true;
		}
		else if (button == JoystickButtonDpadRight && mControllerStatesCurrent[i].buttonDpadRight == ButtonStatePressed)
		{
			return true;
		}
		else if (button == JoystickButtonDpadDown && mControllerStatesCurrent[i].buttonDpadDown == ButtonStatePressed)
		{
			return true;
		}
		else if (button == JoystickButtonDpadLeft && mControllerStatesCurrent[i].buttonDpadLeft == ButtonStatePressed)
		{
			return true;
		}
		else if (button == JoystickButtonLeftTrigger && mControllerStatesCurrent[i].buttonLeftTrigger == ButtonStatePressed)
		{
			return true;
		}
		else if (button == JoystickButtonRightTrigger && mControllerStatesCurrent[i].buttonRightTrigger == ButtonStatePressed)
		{
			return true;
		}
	}
	return false;
}

bool inputManager::buttonPressed(JoystickButton button, int port)
{
    // Controller processing

	for (int i = 0; i < XGetPortCount(); i++)
	{
		if (port >= 0 && port != i)
		{
			continue;
		}

		if (mControllerHandles[i] == NULL)
		{
			continue;
		}

		if (button == JoystickButtonA && mControllerStatesCurrent[i].buttonA == ButtonStatePressed && mControllerStatesPrevious[i].buttonA == ButtonStateNone)
		{
			return true;
		}
		else if (button == JoystickButtonB && mControllerStatesCurrent[i].buttonB == ButtonStatePressed && mControllerStatesPrevious[i].buttonB == ButtonStateNone)
		{
			return true;
		}
		else if (button == JoystickButtonX && mControllerStatesCurrent[i].buttonX == ButtonStatePressed && mControllerStatesPrevious[i].buttonX == ButtonStateNone)
		{
			return true;
		}
		else if (button == JoystickButtonY && mControllerStatesCurrent[i].buttonY == ButtonStatePressed && mControllerStatesPrevious[i].buttonY == ButtonStateNone)
		{
			return true;
		}
		else if (button == JoystickButtonWhite && mControllerStatesCurrent[i].buttonWhite == ButtonStatePressed && mControllerStatesPrevious[i].buttonWhite == ButtonStateNone)
		{
			return true;
		}
		else if (button == JoystickButtonBlack && mControllerStatesCurrent[i].buttonBlack == ButtonStatePressed && mControllerStatesPrevious[i].buttonBlack == ButtonStateNone)
		{
			return true;
		}
		else if (button == JoystickButtonBack && mControllerStatesCurrent[i].buttonBack == ButtonStatePressed && mControllerStatesPrevious[i].buttonBack == ButtonStateNone)
		{
			return true;
		}
		else if (button == JoystickButtonStart && mControllerStatesCurrent[i].buttonStart == ButtonStatePressed && mControllerStatesPrevious[i].buttonStart == ButtonStateNone)
		{
			return true;
		}
		else if (button == JoystickButtonLeftThumb && mControllerStatesCurrent[i].buttonLeftThumb == ButtonStatePressed && mControllerStatesPrevious[i].buttonLeftThumb == ButtonStateNone)
		{
			return true;
		}
		else if (button == JoystickButtonRightThumb && mControllerStatesCurrent[i].buttonRightThumb == ButtonStatePressed && mControllerStatesPrevious[i].buttonRightThumb == ButtonStateNone)
		{
			return true;
		}
		else if (button == JoystickButtonDpadUp && mControllerStatesCurrent[i].buttonDpadUp == ButtonStatePressed && mControllerStatesPrevious[i].buttonDpadUp == ButtonStateNone)
		{
			return true;
		}
		else if (button == JoystickButtonDpadRight && mControllerStatesCurrent[i].buttonDpadRight == ButtonStatePressed && mControllerStatesPrevious[i].buttonDpadRight == ButtonStateNone)
		{
			return true;
		}
		else if (button == JoystickButtonDpadDown && mControllerStatesCurrent[i].buttonDpadDown == ButtonStatePressed && mControllerStatesPrevious[i].buttonDpadDown == ButtonStateNone)
		{
			return true;
		}
		else if (button == JoystickButtonDpadLeft && mControllerStatesCurrent[i].buttonDpadLeft == ButtonStatePressed && mControllerStatesPrevious[i].buttonDpadLeft == ButtonStateNone)
		{
			return true;
		}
		else if (button == JoystickButtonLeftTrigger && mControllerStatesCurrent[i].buttonLeftTrigger == ButtonStatePressed && mControllerStatesPrevious[i].buttonLeftTrigger == ButtonStateNone)
		{
			return true;
		}
		else if (button == JoystickButtonRightTrigger && mControllerStatesCurrent[i].buttonRightTrigger == ButtonStatePressed && mControllerStatesPrevious[i].buttonRightTrigger == ButtonStateNone)
		{
			return true;
		}
	}

    // Remote processing

    for (int i = 0; i < XGetPortCount(); i++)
	{
		if (port >= 0 && port != i)
		{
			continue;
		}

		if (mRemoteHandles[i] == NULL)
		{
			continue;
		}

		if (button == JoystickButtonA && mRemoteStatesCurrent[i] == XINPUT_IR_REMOTE_A1 && mRemoteStatesPrevious[i] == 0)
		{
			return true;
		}
		if (button == JoystickButtonA && mRemoteStatesCurrent[i] == XINPUT_IR_REMOTE_A2 && mRemoteStatesPrevious[i] == 0)
		{
			return true;
		}
		else if (button == JoystickButtonB && mRemoteStatesCurrent[i] == XINPUT_IR_REMOTE_B1 && mRemoteStatesPrevious[i] == 0)
		{
            return true;
		}
		else if (button == JoystickButtonX && mRemoteStatesCurrent[i] == XINPUT_IR_REMOTE_X1 && mRemoteStatesPrevious[i] == 0)
		{
			return true;
		}
        else if (button == JoystickButtonX && mRemoteStatesCurrent[i] == XINPUT_IR_REMOTE_X2 && mRemoteStatesPrevious[i] == 0)
		{
			return true;
		}
		else if (button == JoystickButtonY && mRemoteStatesCurrent[i] == XINPUT_IR_REMOTE_Y1 && mRemoteStatesPrevious[i] == 0)
		{
			return true;
		}
		else if (button == JoystickButtonWhite && mRemoteStatesCurrent[i] == XINPUT_IR_REMOTE_WHITE && mRemoteStatesPrevious[i] == 0)
		{
			return true;
		}
		else if (button == JoystickButtonBlack && mRemoteStatesCurrent[i] == XINPUT_IR_REMOTE_BLACK && mRemoteStatesPrevious[i] == 0)
		{
			return true;
		}
		else if (button == JoystickButtonLeftThumb && mRemoteStatesCurrent[i] == XINPUT_IR_REMOTE_LEFT_THUMB && mRemoteStatesPrevious[i] == 0)
		{
			return true;
		}
		else if (button == JoystickButtonRightThumb && mRemoteStatesCurrent[i] == XINPUT_IR_REMOTE_RIGHT_THUMB && mRemoteStatesPrevious[i] == 0)
		{
			return true;
		}
		else if (button == JoystickButtonDpadUp && mRemoteStatesCurrent[i] == XINPUT_IR_REMOTE_DPAD_UP && mRemoteStatesPrevious[i] == 0)
		{
			return true;
		}
		else if (button == JoystickButtonDpadRight && mRemoteStatesCurrent[i] == XINPUT_IR_REMOTE_DPAD_RIGHT && mRemoteStatesPrevious[i] == 0)
		{
			return true;
		}
		else if (button == JoystickButtonDpadDown && mRemoteStatesCurrent[i] == XINPUT_IR_REMOTE_DPAD_DOWN && mRemoteStatesPrevious[i] == 0)
		{
			return true;
		}
		else if (button == JoystickButtonDpadLeft && mRemoteStatesCurrent[i] == XINPUT_IR_REMOTE_DPAD_LEFT && mRemoteStatesPrevious[i] == 0)
		{
			return true;
		}
		else if (button == JoystickButtonLeftTrigger && mRemoteStatesCurrent[i] == XINPUT_IR_REMOTE_LEFT_TRIGGER && mRemoteStatesPrevious[i] == 0)
		{
			return true;
		}
		else if (button == JoystickButtonRightTrigger && mRemoteStatesCurrent[i] == XINPUT_IR_REMOTE_RIGHT_TRIGGER && mRemoteStatesPrevious[i] == 0)
		{
			return true;
		}
	}
	return false;
}

BYTE inputManager::getLeftTriggerPressure(int port)
{
    for (int i = 0; i < XGetPortCount(); i++)
    {
        if (port >= 0 && port != i) continue;
        if (mControllerHandles[i] == NULL) continue;
        return mTriggerLeftPressure[i];
    }
    return 0;
}

BYTE inputManager::getRightTriggerPressure(int port)
{
    for (int i = 0; i < XGetPortCount(); i++)
    {
        if (port >= 0 && port != i) continue;
        if (mControllerHandles[i] == NULL) continue;
        return mTriggerRightPressure[i];
    }
    return 0;
}