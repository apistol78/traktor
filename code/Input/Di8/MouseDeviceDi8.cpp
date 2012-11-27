#include "Core/Log/Log.h"
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
	{ L"Aux mouse button", DtButton3, false, true, 3 },
	{ L"Mouse X axis", DtAxisX, true, true, 4 },
	{ L"Mouse Y axis", DtAxisY, true, true, 5 },
	{ L"Mouse Z axis", DtAxisZ, true, true, 6 },
	{ L"Mouse X axis", DtPositionX, true, false, 7 },
	{ L"Mouse Y axis", DtPositionY, true, false, 8 }
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.MouseDeviceDi8", MouseDeviceDi8, IInputDevice)

MouseDeviceDi8::MouseDeviceDi8(HWND hWnd, IDirectInputDevice8* device, const DIDEVICEINSTANCE* deviceInstance)
:	m_hWnd(hWnd)
,	m_device(device)
,	m_connected(false)
,	m_exclusive(false)
{
	m_device->SetDataFormat(&c_dfDIMouse);
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
		return float(m_state.lX);
	case 5:
		return float(m_state.lY);
	case 6:
		return float(m_state.lZ / 120.0f);
	case 7:
		return float(m_position.x);
	case 8:
		return float(m_position.y);
	}

	return 0.0f;
}

bool MouseDeviceDi8::getControlRange(int32_t control, float& outMin, float& outMax) const
{
	const MouseControlMap& mc = c_mouseControlMap[control];
	if (mc.index == 7)
	{
		outMin = float(m_rect.left);
		outMax = float(m_rect.right);
		return true;
	}
	else if (mc.index == 8)
	{
		outMin = float(m_rect.top);
		outMax = float(m_rect.bottom);
		return true;
	}
	else if (mc.index <= 3)
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

	hr = m_device->GetDeviceState(sizeof(DIMOUSESTATE), &m_state);

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

	// As long as mouse is exclusivly acquired we clamp mouse position to client area of window.
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
	// Ensure device is unaquired, cannot change cooperative level if acquired.
	m_device->Unacquire();
	m_connected = false;
	m_exclusive = exclusive;
}

	}
}
