#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Input/Di8/KeyboardDeviceDi8.h"
#include "Input/Di8/TypesDi8.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.KeyboardDeviceDi8", KeyboardDeviceDi8, IInputDevice)

KeyboardDeviceDi8::KeyboardDeviceDi8(IDirectInputDevice8* device, const DIDEVICEINSTANCE* deviceInstance)
:	m_device(device)
,	m_connected(false)
{
	m_device->SetDataFormat(&c_dfDIKeyboard);
	m_name = tstows(deviceInstance->tszInstanceName);

	HRESULT hr = device->Acquire();
	m_connected = SUCCEEDED(hr);
}

std::wstring KeyboardDeviceDi8::getName() const
{
	return m_name;
}

InputCategory KeyboardDeviceDi8::getCategory() const
{
	return CtKeyboard;
}

bool KeyboardDeviceDi8::isConnected() const
{
	return m_connected;
}

int32_t KeyboardDeviceDi8::getControlCount()
{
	return sizeof_array(c_di8ControlKeys);
}

std::wstring KeyboardDeviceDi8::getControlName(int32_t control)
{
	return L"";
}

bool KeyboardDeviceDi8::isControlAnalogue(int32_t control) const
{
	return false;
}

int32_t KeyboardDeviceDi8::getActiveControlCount() const
{
	return 0;
}

float KeyboardDeviceDi8::getControlValue(int32_t control)
{
	if (!m_connected)
		return 0.0f;

	DWORD dik = c_di8ControlKeys[control];
	if (dik == 0)
		return 0.0f;

	return (m_state[dik] & 0x80) ? 1.0f : 0.0f;
}

bool KeyboardDeviceDi8::getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const
{
	if (analogue)
		return false;

	control = controlType;
	return c_di8ControlKeys[control] != 0;
}

void KeyboardDeviceDi8::resetState()
{
	std::memset(&m_state, 0, sizeof(m_state));
}

void KeyboardDeviceDi8::readState()
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

	hr = m_device->GetDeviceState(256, m_state);

	m_connected = SUCCEEDED(hr);	
}

bool KeyboardDeviceDi8::supportRumble() const
{
	return false;
}

void KeyboardDeviceDi8::setRumble(const InputRumble& rumble)
{
}

	}
}
