#include <cstring>
#include "Core/Misc/TString.h"
#include "Input/X11/KeyboardDeviceX11.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.KeyboardDeviceX11", KeyboardDeviceX11, IInputDevice)

KeyboardDeviceX11::KeyboardDeviceX11()
:	m_connected(true)
{
	resetState();
}

std::wstring KeyboardDeviceX11::getName() const
{
	return L"Standard Keyboard";
}

InputCategory KeyboardDeviceX11::getCategory() const
{
	return CtKeyboard;
}

bool KeyboardDeviceX11::isConnected() const
{
	return m_connected;
}

int32_t KeyboardDeviceX11::getControlCount()
{
	return sizeof_array(m_keyStates);
}

std::wstring KeyboardDeviceX11::getControlName(int32_t control)
{
	return L"";
}

bool KeyboardDeviceX11::isControlAnalogue(int32_t control) const
{
	return false;
}

bool KeyboardDeviceX11::isControlStable(int32_t control) const
{
	return false;
}

float KeyboardDeviceX11::getControlValue(int32_t control)
{
	if (m_connected)
		return m_keyStates[control] ? 1.0f : 0.0f;
	else
		return 0.0f;
}

bool KeyboardDeviceX11::getControlRange(int32_t control, float& outMin, float& outMax) const
{
	outMin = 0.0f;
	outMax = 1.0f;
	return true;
}

bool KeyboardDeviceX11::getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const
{
//	if (analogue || !c_vkControlKeys[int32_t(controlType)])
//		return false;

	control = int32_t(controlType);
	return true;
}

bool KeyboardDeviceX11::getKeyEvent(KeyEvent& outEvent)
{
	if (m_keyEvents.empty())
		return false;

	outEvent = m_keyEvents.front();
	m_keyEvents.pop_front();

	return true;
}

void KeyboardDeviceX11::resetState()
{
	std::memset(m_keyStates, 0, sizeof(m_keyStates));
	m_keyEvents.clear();
}

void KeyboardDeviceX11::readState()
{
	if (m_connected)
	{
		/*
		for (int32_t i = 0; i < sizeof_array(c_vkControlKeys); ++i)
		{
			if (c_vkControlKeys[i] == 0)
				continue;

			SHORT keyState = GetAsyncKeyState(c_vkControlKeys[i]);
			if (keyState & 0x8000)
				m_keyStates[i] = 0xff;
			else
				m_keyStates[i] = 0x00;
		}
		*/
	}
	else
		resetState();
}

bool KeyboardDeviceX11::supportRumble() const
{
	return false;
}

void KeyboardDeviceX11::setRumble(const InputRumble& /*rumble*/)
{
}

void KeyboardDeviceX11::setExclusive(bool exclusive)
{
}

	}
}
