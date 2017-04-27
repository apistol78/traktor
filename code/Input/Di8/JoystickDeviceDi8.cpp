/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Input/Di8/JoystickDeviceDi8.h"
#include "Input/Di8/TypesDi8.h"

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

bool povInRange(DWORD pov, DWORD mn, DWORD mx)
{
	if (LOWORD(pov) == 0xffff)
		return false;
	else
		return pov >= mn && pov <= mx;
}

template < typename T >
T readStateValueByOffset(const DIJOYSTATE2& state, uint32_t offset)
{
	T_ASSERT (offset + sizeof(T) <= sizeof(DIJOYSTATE2));
	return *(T*)(((uint8_t*)&state) + offset);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.JoystickDeviceDi8", JoystickDeviceDi8, IInputDevice)

JoystickDeviceDi8::JoystickDeviceDi8(HWND hWnd, IDirectInputDevice8* device, const DIDEVICEINSTANCE* deviceInstance)
:	m_hWnd(hWnd)
,	m_device(device)
,	m_connected(false)
{
	m_device->SetDataFormat(&c_dfDIJoystick2);
	m_name = tstows(deviceInstance->tszInstanceName);

	collectControls(device);

	HRESULT hr = device->Acquire();
	m_connected = SUCCEEDED(hr);
}

std::wstring JoystickDeviceDi8::getName() const
{
	return m_name;
}

InputCategory JoystickDeviceDi8::getCategory() const
{
	return CtJoystick;
}

bool JoystickDeviceDi8::isConnected() const
{
	return m_connected;
}

int32_t JoystickDeviceDi8::getControlCount()
{
	return int32_t(m_controlInfo.size());
}

std::wstring JoystickDeviceDi8::getControlName(int32_t control)
{
	return m_controlInfo[control].name;
}

bool JoystickDeviceDi8::isControlAnalogue(int32_t control) const
{
	return m_controlInfo[control].analogue;
}

bool JoystickDeviceDi8::isControlStable(int32_t control) const
{
	return true;
}

float JoystickDeviceDi8::getControlValue(int32_t control)
{
	if (!m_connected)
		return 0.0f;

	const ControlInfo& controlInfo = m_controlInfo[control];

	switch (controlInfo.controlType)
	{
	case DtUp:
		if (
			povInRange(m_state.rgdwPOV[0], 31500, 36000) ||
			povInRange(m_state.rgdwPOV[0], 0, 4500)
		)
			return 1.0f;
		else
			return 0.0f;

	case DtDown:
		if (povInRange(m_state.rgdwPOV[0], 13500, 22500))
			return 1.0f;
		else
			return 0.0f;

	case DtLeft:
		if (povInRange(m_state.rgdwPOV[0], 22500, 31500))
			return 1.0f;
		else
			return 0.0f;

	case DtRight:
		if (povInRange(m_state.rgdwPOV[0], 4500, 13500))
			return 1.0f;
		else
			return 0.0f;

	case DtThumbLeftX:
	case DtThumbLeftY:
	case DtThumbLeftZ:
	case DtThumbRightX:
	case DtThumbRightY:
	case DtThumbRightZ:
	case DtSliderLeft:
	case DtSliderRight:
		{
			LONG stateValue = readStateValueByOffset< LONG >(m_state, controlInfo.offset);
			float value = adjustDeadZone((stateValue - 32767.0f) / 32767.0f);
			return controlInfo.inverted ? -value : value;
		}

	case DtSelect:
	case DtCancel:
	case DtButton1:
	case DtButton2:
	case DtButton3:
	case DtButton4:
	case DtButton5:
	case DtButton6:
	case DtButton7:
	case DtButton8:
	case DtThumbLeftPush:
	case DtThumbRightPush:
	case DtTriggerLeft:
	case DtTriggerRight:
	case DtShoulderLeft:
	case DtShoulderRight:
		{
			BYTE value = readStateValueByOffset< BYTE >(m_state, controlInfo.offset);
			return ((value & 0x80) != 0) ? 1.0f : 0.0f;
		}
	}

	return 0.0f;
}

bool JoystickDeviceDi8::getControlRange(int32_t control, float& outMin, float& outMax) const
{
	return false;
}

bool JoystickDeviceDi8::getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const
{
	for (int i = 0; i < int(m_controlInfo.size()); ++i)
	{
		if (m_controlInfo[i].controlType == controlType && m_controlInfo[i].analogue == analogue)
		{
			control = i;
			return true;
		}
	}
	return false;
}

bool JoystickDeviceDi8::getKeyEvent(KeyEvent& outEvent)
{
	return false;
}

void JoystickDeviceDi8::resetState()
{
	std::memset(&m_state, 0, sizeof(m_state));
}

void JoystickDeviceDi8::readState()
{
	HRESULT hr;

	if (!m_connected)
	{
		hr = m_device->Acquire();
		m_connected = SUCCEEDED(hr);
		if (!m_connected)
			return;
	}

	hr = m_device->Poll();
	if (FAILED(hr))  
	{
		hr = m_device->Acquire();
		m_connected = SUCCEEDED(hr);
		if (!m_connected)
			return;
	}

	hr = m_device->GetDeviceState(sizeof(DIJOYSTATE2), &m_state);

	m_connected = SUCCEEDED(hr);	
}

bool JoystickDeviceDi8::supportRumble() const
{
	return false;
}

void JoystickDeviceDi8::setRumble(const InputRumble& rumble)
{
}

void JoystickDeviceDi8::setExclusive(bool exclusive)
{
	// Ensure device is unaquired, cannot change cooperative level if acquired.
	m_device->Unacquire();
	m_connected = false;

	// Change cooperative level.
	HRESULT hr = m_device->SetCooperativeLevel(m_hWnd, exclusive ? (DISCL_FOREGROUND | DISCL_EXCLUSIVE) : (DISCL_FOREGROUND | DISCL_NONEXCLUSIVE));
	if (FAILED(hr))
		log::warning << L"Unable to set cooperative level on joystick device" << Endl;
}

void JoystickDeviceDi8::collectControls(IDirectInputDevice8* device)
{
	struct ControlTemplate
	{
		InputDefaultControlType controlType;
		uint32_t offset;
		bool analogue;
		bool inverted;
	}
	c_controlTemplates[] =
	{
		{ DtUp, DIJOFS_POV(0), false, false },
		{ DtDown, DIJOFS_POV(0), false, false },
		{ DtLeft, DIJOFS_POV(0), false, false },
		{ DtRight, DIJOFS_POV(0), false, false },
		{ DtSelect, DIJOFS_BUTTON(9), false, false },
		{ DtCancel, DIJOFS_BUTTON(8), false, false },
		{ DtThumbLeftX, DIJOFS_X, true, false },
		{ DtThumbLeftY, DIJOFS_Y, true, true },
		{ DtThumbLeftZ, DIJOFS_Z, true, false },
		{ DtThumbLeftPush, DIJOFS_BUTTON(14), false, false },
		{ DtThumbRightX, DIJOFS_RX, true, false },
		{ DtThumbRightY, DIJOFS_RY, true, true },
		{ DtThumbRightZ, DIJOFS_RZ, true, false },
		{ DtThumbRightPush, DIJOFS_BUTTON(15), false, false },
		{ DtSliderLeft, DIJOFS_SLIDER(0), true, false },
		{ DtSliderRight, DIJOFS_SLIDER(1), true, false },
		{ DtTriggerLeft, DIJOFS_BUTTON(6), false, false },
		{ DtTriggerRight, DIJOFS_BUTTON(7), false, false },
		{ DtShoulderLeft, DIJOFS_BUTTON(4), false, false },
		{ DtShoulderRight, DIJOFS_BUTTON(5), false, false },
		{ DtButton1, DIJOFS_BUTTON(0), false, false },
		{ DtButton2, DIJOFS_BUTTON(1), false, false },
		{ DtButton3, DIJOFS_BUTTON(2), false, false },
		{ DtButton4, DIJOFS_BUTTON(3), false, false },
		{ DtButton5, DIJOFS_BUTTON(10), false, false },
		{ DtButton6, DIJOFS_BUTTON(11), false, false },
		{ DtButton7, DIJOFS_BUTTON(12), false, false },
		{ DtButton8, DIJOFS_BUTTON(13), false, false }
	};

	int32_t sliderIndex = 0;
	DIDEVICEOBJECTINSTANCE didoi;
	HRESULT hr;

	for (uint32_t i = 0; i < sizeof_array(c_controlTemplates); i++)
	{
		didoi.dwSize = sizeof(DIDEVICEOBJECTINSTANCE);

		hr = device->GetObjectInfo(&didoi, c_controlTemplates[i].offset, DIPH_BYOFFSET);
		if (FAILED(hr))
			continue;

		ControlInfo controlInfo;
		controlInfo.name = tstows(didoi.tszName);
		controlInfo.controlType = c_controlTemplates[i].controlType;
		controlInfo.offset = c_controlTemplates[i].offset;
		controlInfo.analogue = c_controlTemplates[i].analogue;
		controlInfo.inverted = c_controlTemplates[i].inverted;
		m_controlInfo.push_back(controlInfo);
	}
}

	}
}
