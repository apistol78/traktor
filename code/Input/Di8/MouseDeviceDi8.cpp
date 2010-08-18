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
	return 0;
}

std::wstring MouseDeviceDi8::getControlName(int32_t control)
{
	return L"";
}

bool MouseDeviceDi8::isControlAnalogue(int32_t control) const
{
	return false;
}

float MouseDeviceDi8::getControlValue(int32_t control)
{
	if (!m_connected)
		return 0.0f;

	// \fixme

	return 0.0f;
}

bool MouseDeviceDi8::getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const
{
	control = controlType;
	return true;
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
