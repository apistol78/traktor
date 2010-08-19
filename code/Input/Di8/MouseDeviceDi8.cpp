#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Input/Di8/MouseDeviceDi8.h"
#include "Input/Di8/TypesDi8.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.MouseDeviceDi8", MouseDeviceDi8, IInputDevice)

MouseDeviceDi8::MouseDeviceDi8(IDirectInputDevice8* device, const DIDEVICEINSTANCE* deviceInstance)
:	m_device(device)
,	m_connected(false)
{
	m_device->SetDataFormat(&c_dfDIMouse);
	m_name = tstows(deviceInstance->tszInstanceName);
	
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
	return 7;
}

std::wstring MouseDeviceDi8::getControlName(int32_t control)
{
	switch (control)
	{
	case 1:
		return L"Axis X";
	case 2:
		return L"Axis Y";
	case 3:
		return L"Wheel";
	case 4:
		return L"Left button";
	case 5:
		return L"Right button";
	case 6:
		return L"Middle button";
	case 7:
		return L"Aux button";
	}
	return L"";
}

bool MouseDeviceDi8::isControlAnalogue(int32_t control) const
{
	return control < 0;
}

float MouseDeviceDi8::getControlValue(int32_t control)
{
	if (!m_connected)
		return 0.0f;

	switch (control)
	{
	case 1:
		return float(m_state.lX);
	case 2:
		return float(m_state.lY);
	case 3:
		return float(m_state.lZ / 120.0f);
	case 4:
		return (m_state.rgbButtons[0] & 0x80) ? 1.0f : 0.0f;
	case 5:
		return (m_state.rgbButtons[1] & 0x80) ? 1.0f : 0.0f;
	case 6:
		return (m_state.rgbButtons[2] & 0x80) ? 1.0f : 0.0f;
	case 7:
		return (m_state.rgbButtons[3] & 0x80) ? 1.0f : 0.0f;
	}

	return 0.0f;
}

bool MouseDeviceDi8::getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const
{
	control = 0;
	switch (controlType)
	{
	case DtAxisX:
		if (analogue)
			control = 1;
		break;

	case DtAxisY:
		if (analogue)
			control = 2;
		break;

	case DtAxisZ:
		if (analogue)
			control = 3;
		break;

	case DtButton1:
		if (!analogue)
			control = 4;
		break;

	case DtButton2:
		if (!analogue)
			control = 5;
		break;

	case DtButton3:
		if (!analogue)
			control = 6;
		break;

	case DtButton4:
		if (!analogue)
			control = 7;
		break;
	}
	return control != 0;
}

void MouseDeviceDi8::resetState()
{
	std::memset(&m_state, 0, sizeof(m_state));
}

void MouseDeviceDi8::readState()
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

	hr = m_device->GetDeviceState(sizeof(DIMOUSESTATE), &m_state);

	m_connected = SUCCEEDED(hr);	
}

bool MouseDeviceDi8::supportRumble() const
{
	return false;
}

void MouseDeviceDi8::setRumble(const InputRumble& rumble)
{
}

	}
}
