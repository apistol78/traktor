#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Input/Di8/MouseDeviceDi8.h"
#include "Input/Di8/TypesDi8.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

const struct MouseControlMap
{
	const wchar_t* name;
	InputDefaultControlType controlType;
	bool analogue;
	bool stable;
	int32_t index;
}
c_mouseControlMap[] =
{
	{ L"Left mouse button", DtButton1, false, true, 0 },
	{ L"Right mouse button", DtButton2, false, true, 1 },
	{ L"Middle mouse button", DtButton3, false, true, 2 },
	{ L"Aux 1 mouse button", DtButton3, false, true, 3 },
	{ L"Aux 2 mouse button", DtButton4, false, true, 4 },
	{ L"Aux 3 mouse button", DtButton5, false, true, 5 },
	{ L"Aux 4 mouse button", DtButton6, false, true, 6 },
	{ L"Aux 5 mouse button", DtButton7, false, true, 7 },
	{ L"Mouse X axis", DtAxisX, true, false, 8 },
	{ L"Mouse Y axis", DtAxisY, true, false, 9 },
	{ L"Mouse Z axis", DtAxisZ, true, false, 10 },
	{ L"Mouse X axis", DtPositionX, true, false, 11 },
	{ L"Mouse Y axis", DtPositionY, true, false, 12 }
};

const float c_mouseDeltaLimit = 100.0f;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.MouseDeviceDi8", MouseDeviceDi8, IInputDevice)

MouseDeviceDi8::MouseDeviceDi8(HWND hWnd, IDirectInputDevice8* device, const DIDEVICEINSTANCE* deviceInstance)
:	m_hWnd(hWnd)
,	m_device(device)
,	m_connected(false)
,	m_exclusive(false)
{
	m_device->SetDataFormat(&c_dfDIMouse2);
	m_name = tstows(deviceInstance->tszInstanceName);
	
	resetState();

	HRESULT hr = device->Acquire();
	m_connected = SUCCEEDED(hr);
}

std::wstring MouseDeviceDi8::getName() const
{
	return m_name;
}

InputCategory MouseDeviceDi8::getCategory() const
{
	return CtMouse;
}

bool MouseDeviceDi8::isConnected() const
{
	return m_connected;
}

int32_t MouseDeviceDi8::getControlCount()
{
	return sizeof_array(c_mouseControlMap);
}

std::wstring MouseDeviceDi8::getControlName(int32_t control)
{
	return c_mouseControlMap[control].name;
}

bool MouseDeviceDi8::isControlAnalogue(int32_t control) const
{
	return c_mouseControlMap[control].analogue;
}

bool MouseDeviceDi8::isControlStable(int32_t control) const
{
	return c_mouseControlMap[control].stable;
}

float MouseDeviceDi8::getControlValue(int32_t control)
{
	if (!m_connected)
		return 0.0f;

	const MouseControlMap& mc = c_mouseControlMap[control];
	switch (mc.index)
	{
	case 0:
		return (m_state.rgbButtons[0] & 0x80) ? 1.0f : 0.0f;
	case 1:
		return (m_state.rgbButtons[1] & 0x80) ? 1.0f : 0.0f;
	case 2:
		return (m_state.rgbButtons[2] & 0x80) ? 1.0f : 0.0f;
	case 3:
		return (m_state.rgbButtons[3] & 0x80) ? 1.0f : 0.0f;
	case 4:
		return (m_state.rgbButtons[4] & 0x80) ? 1.0f : 0.0f;
	case 5:
		return (m_state.rgbButtons[5] & 0x80) ? 1.0f : 0.0f;
	case 6:
		return (m_state.rgbButtons[6] & 0x80) ? 1.0f : 0.0f;
	case 7:
		return (m_state.rgbButtons[7] & 0x80) ? 1.0f : 0.0f;
	case 8:
		return clamp(float(m_state.lX), -c_mouseDeltaLimit, c_mouseDeltaLimit);
	case 9:
		return clamp(float(m_state.lY), -c_mouseDeltaLimit, c_mouseDeltaLimit);
	case 10:
		return float(m_state.lZ / 120.0f);
	case 11:
		return float(m_position.x);
	case 12:
		return float(m_position.y);
	}

	return 0.0f;
}

bool MouseDeviceDi8::getControlRange(int32_t control, float& outMin, float& outMax) const
{
	const MouseControlMap& mc = c_mouseControlMap[control];
	if (mc.index == 8 || mc.index == 9)
	{
		outMin = -100.0f;
		outMax =  100.0f;
		return true;
	}
	else if (mc.index == 11)
	{
		outMin = float(m_rect.left);
		outMax = float(m_rect.right);
		return true;
	}
	else if (mc.index == 12)
	{
		outMin = float(m_rect.top);
		outMax = float(m_rect.bottom);
		return true;
	}
	else if (mc.index <= 7)
	{
		outMin = 0.0f;
		outMax = 1.0f;
		return true;
	}
	else
		return false;
}

bool MouseDeviceDi8::getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const
{
	for (uint32_t i = 0; i < sizeof_array(c_mouseControlMap); ++i)
	{
		const MouseControlMap& mc = c_mouseControlMap[i];
		if (mc.controlType == controlType && mc.analogue == analogue)
		{
			control = i;
			return true;
		}
	}
	return false;
}

bool MouseDeviceDi8::getKeyEvent(KeyEvent& outEvent)
{
	return false;
}

void MouseDeviceDi8::resetState()
{
	std::memset(&m_state, 0, sizeof(m_state));
	std::memset(&m_position, 0, sizeof(m_position));
}

void MouseDeviceDi8::readState()
{
	HRESULT hr;

	if (!m_connected)
	{
		hr = m_device->SetCooperativeLevel(m_hWnd, m_exclusive ? (DISCL_FOREGROUND | DISCL_EXCLUSIVE) : (DISCL_FOREGROUND | DISCL_NONEXCLUSIVE));
		if (FAILED(hr))
			return;

		hr = m_device->Acquire();
		m_connected = SUCCEEDED(hr);
		if (!m_connected)
			return;
	}

	hr = m_device->Poll();
	if (FAILED(hr))  
	{
		m_device->Unacquire();
		m_connected = false;
		if (!m_connected)
			return;
	}

	hr = m_device->GetDeviceState(sizeof(DIMOUSESTATE2), &m_state);

	GetCursorPos(&m_position);

	HWND hWndActive = GetActiveWindow();
	ScreenToClient(hWndActive, &m_position);
	GetClientRect(hWndActive, &m_rect);

	if (m_position.x < 0)
		m_position.x = 0;
	else if (m_position.x > m_rect.right - 1)
		m_position.x = m_rect.right - 1;

	if (m_position.y < 0)
		m_position.y = 0;
	else if (m_position.y > m_rect.bottom - 1)
		m_position.y = m_rect.bottom - 1;

	m_connected = SUCCEEDED(hr);	

	// As long as mouse is exclusively acquired we clamp mouse position to client area of window.
	if (m_exclusive && m_connected)
	{
		POINT position = m_position;
		ClientToScreen(hWndActive, &position);
		SetCursorPos(position.x, position.y);
	}
}

bool MouseDeviceDi8::supportRumble() const
{
	return false;
}

void MouseDeviceDi8::setRumble(const InputRumble& rumble)
{
}

void MouseDeviceDi8::setExclusive(bool exclusive)
{
	// Ensure device is unacquired, cannot change cooperative level if acquired.
	m_device->Unacquire();
	m_connected = false;
	m_exclusive = exclusive;
}

	}
}
