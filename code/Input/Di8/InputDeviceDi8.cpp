#include "Input/Di8/InputDeviceDi8.h"
#include "Core/Misc/String.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

float adjustDeadZone(float value)
{
	if (value >= -0.2f && value <= 0.2f)
		value = 0.0f;
	return value;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputDeviceDi8", InputDeviceDi8, IInputDevice)

InputDeviceDi8::InputDeviceDi8(IDirectInputDevice8* device)
:	m_device(device)
{
	HRESULT hr;
	
	memset(&m_deviceDesc, 0, sizeof(DIDEVICEINSTANCE));
	m_deviceDesc.dwSize = sizeof(DIDEVICEINSTANCE);

	hr = device->GetDeviceInfo(&m_deviceDesc);
	T_ASSERT(SUCCEEDED(hr));

	switch (getCategory())
	{
	case CtMouse:
		device->SetDataFormat(&c_dfDIMouse);
		m_state = new DIMOUSESTATE;
		break;

	case CtKeyboard:
		device->SetDataFormat(&c_dfDIKeyboard);
		m_state = new uint8_t [256];
		break;

	case CtJoystick:
		device->SetDataFormat(&c_dfDIJoystick2);
		m_state = new DIJOYSTATE2;
		break;
	}

	hr = device->Acquire();
	T_ASSERT(SUCCEEDED(hr));
}

std::wstring InputDeviceDi8::getName() const
{
	return m_deviceDesc.tszInstanceName;
}

InputCategory InputDeviceDi8::getCategory() const
{
	switch (m_deviceDesc.dwDevType & 0xff)
	{
	case DI8DEVTYPE_1STPERSON:
	case DI8DEVTYPE_MOUSE:
		return CtMouse;
		break;

	case DI8DEVTYPE_KEYBOARD:
		return CtKeyboard;
		break;

	case DI8DEVTYPE_JOYSTICK:
	case DI8DEVTYPE_GAMEPAD:
	case DI8DEVTYPE_DRIVING:
	case DI8DEVTYPE_FLIGHT:
	default:
		return CtJoystick;
		break;
	}
}

bool InputDeviceDi8::isConnected() const
{
	return true;
}

int InputDeviceDi8::getControlCount()
{
	switch (getCategory())
	{
	case CtMouse:
		return 0;	// TODO: Fix me

	case CtKeyboard:
		return 256;

	case CtJoystick:
		return 0;	// TODO: Fix me
	}
	return 0;
}

std::wstring InputDeviceDi8::getControlName(int control)
{
	return L"";
}

bool InputDeviceDi8::isControlAnalogue(int control) const
{
	return false;
}

float InputDeviceDi8::getControlValue(int control)
{
#define KEYDOWN(name, key) (name[key] & 0x80)

	switch (getCategory())
	{
	case CtMouse:
		break;

	case CtKeyboard:
		{
			const uint8_t* state = static_cast< const uint8_t* >(m_state);
			switch (control)
			{
			case DtUp:
				return (state[DIK_W] & 0x80) ? 1.0f : 0.0f;

			case DtDown:
				return (state[DIK_S] & 0x80) ? 1.0f : 0.0f;

			case DtLeft:
				return (state[DIK_A] & 0x80) ? 1.0f : 0.0f;

			case DtRight:
				return (state[DIK_D] & 0x80) ? 1.0f : 0.0f;

			case DtSelect:
				return (state[DIK_RETURN] & 0x80) ? 1.0f : 0.0f;

			case DtCancel:
				return (state[DIK_ESCAPE] & 0x80) ? 1.0f : 0.0f;

			case DtButton1:
				return (state[DIK_1] & 0x80) ? 1.0f : 0.0f;

			case DtButton2:
				return (state[DIK_2] & 0x80) ? 1.0f : 0.0f;

			case DtButton3:
				return (state[DIK_3] & 0x80) ? 1.0f : 0.0f;

			case DtButton4:
				return (state[DIK_4] & 0x80) ? 1.0f : 0.0f;
			}
		}
		break;

	case CtJoystick:
		{
			DIJOYSTATE* state = static_cast< DIJOYSTATE* >(m_state);
			switch (control)
			{
			case DtUp:
				return (state->rgbButtons[3] == 0.0f) ? 0.0f : 1.0f;

			case DtDown:
				return (state->rgbButtons[1] == 0.0f) ? 0.0f : 1.0f;

			case DtLeft:
				return (state->rgbButtons[0] == 0.0f) ? 0.0f : 1.0f;

			case DtRight:
				return (state->rgbButtons[2] == 0.0f) ? 0.0f : 1.0f;

			case DtSelect:
				return (state->rgbButtons[9] == 0.0f) ? 0.0f : 1.0f;

			case DtCancel:
				return (state->rgbButtons[10] == 0.0f) ? 0.0f : 1.0f;

			case DtThumbLeftX:
				return adjustDeadZone((state->lX - 32767.0f) / 32767.0f);

			case DtThumbLeftY:
				return adjustDeadZone(-(state->lY - 32767.0f) / 32767.0f);

			case DtThumbRightX:
				return adjustDeadZone((state->lZ - 32767.0f) / 32767.0f);

			case DtThumbRightY:
				return adjustDeadZone((state->lRz - 32767.0f) / 32767.0f);
			}
		}
		break;
	}

	return 0.0f;
}

bool InputDeviceDi8::getDefaultControl(InputDefaultControlType controlType, int& control) const
{
	control = controlType;
	return true;
}

void InputDeviceDi8::resetState()
{
}

void InputDeviceDi8::readState()
{
	HRESULT hr;

	hr = m_device->Poll();
	if (FAILED(hr))  
	{
		hr = m_device->Acquire();
		if (FAILED(hr))
			return;
	}

	switch (getCategory())
	{
	case CtMouse:
		hr = m_device->GetDeviceState(sizeof(DIMOUSESTATE), m_state);
		break;

	case CtKeyboard:
		hr = m_device->GetDeviceState(256, m_state);
		break;

	case CtJoystick:
		hr = m_device->GetDeviceState(sizeof(DIJOYSTATE2), m_state);
		break;
	}

	T_ASSERT(SUCCEEDED(hr));
}

bool InputDeviceDi8::supportRumble() const
{
	return false;
}

void InputDeviceDi8::setRumble(const InputRumble& rumble)
{
}

	}
}
