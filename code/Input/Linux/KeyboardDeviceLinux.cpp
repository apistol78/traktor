/*
 * TRAKTOR
 * Copyright (c) 2024-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include <xkbcommon/xkbcommon.h>
#include "Core/Misc/TString.h"
#include "Input/Linux/KeyboardDeviceLinux.h"
#include "Input/Linux/TypesLinux.h"

namespace traktor::input
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.KeyboardDeviceLinux", KeyboardDeviceLinux, IInputDevice)

KeyboardDeviceLinux::KeyboardDeviceLinux()
:	m_connected(true)
,	m_focus(true)
{
	resetState();
}

std::wstring KeyboardDeviceLinux::getName() const
{
	return L"Standard Keyboard";
}

InputCategory KeyboardDeviceLinux::getCategory() const
{
	return InputCategory::Keyboard;
}

bool KeyboardDeviceLinux::isConnected() const
{
	return m_connected;
}

int32_t KeyboardDeviceLinux::getControlCount()
{
	return sizeof_array(m_keyStates);
}

std::wstring KeyboardDeviceLinux::getControlName(int32_t control)
{
	if (control < 0 || control >= (int32_t)sizeof_array(c_linuxControlKeys))
		return L"";

	const xkb_keysym_t ks = c_linuxControlKeys[control];
	if (ks == 0)
		return L"";

	char buf[64];
	if (xkb_keysym_get_name(ks, buf, sizeof(buf)) > 0)
		return mbstows(buf);
	else
		return L"";
}

bool KeyboardDeviceLinux::isControlAnalogue(int32_t control) const
{
	return false;
}

bool KeyboardDeviceLinux::isControlStable(int32_t control) const
{
	return true;
}

float KeyboardDeviceLinux::getControlValue(int32_t control)
{
	if (!m_connected || control < 0 || control >= (int32_t)sizeof_array(m_keyStates))
		return 0.0f;

	return m_keyStates[control] ? 1.0f : 0.0f;
}

bool KeyboardDeviceLinux::getControlRange(int32_t control, float& outMin, float& outMax) const
{
	outMin = 0.0f;
	outMax = 1.0f;
	return true;
}

bool KeyboardDeviceLinux::getDefaultControl(DefaultControl controlType, bool analogue, int32_t& control) const
{
	const int32_t index = int32_t(controlType);
	if (analogue || index < 0 || index >= (int32_t)sizeof_array(c_linuxControlKeys) || !c_linuxControlKeys[index])
		return false;

	control = index;
	return true;
}

bool KeyboardDeviceLinux::getKeyEvent(KeyEvent& outEvent)
{
	if (m_keyEvents.empty())
		return false;

	outEvent = m_keyEvents.front();
	m_keyEvents.pop_front();
	return true;
}

void KeyboardDeviceLinux::resetState()
{
	std::memset(m_keyStates, 0, sizeof(m_keyStates));
	m_keyEvents.clear();
}

void KeyboardDeviceLinux::readState()
{
	if (!m_connected)
		resetState();
}

bool KeyboardDeviceLinux::supportRumble() const
{
	return false;
}

void KeyboardDeviceLinux::setRumble(const InputRumble& /*rumble*/)
{
}

void KeyboardDeviceLinux::setExclusive(bool /*exclusive*/)
{
}

void KeyboardDeviceLinux::setFocus(bool focus)
{
	m_focus = focus;
	m_connected = focus;
	if (!focus)
		resetState();
}

void KeyboardDeviceLinux::onKey(int32_t control, bool down, bool repeat)
{
	if (control < 0 || control >= (int32_t)sizeof_array(m_keyStates))
		return;

	m_keyStates[control] = down ? 0xff : 0x00;

	// Suppress synthetic key-repeat for the initial down/up events (matches the
	// X11 device); character repeats are still delivered via onCharacter.
	if (repeat)
		return;

	KeyEvent ke;
	ke.type = down ? KeyEventType::Down : KeyEventType::Up;
	ke.character = (wchar_t)control;
	m_keyEvents.push_back(ke);
}

void KeyboardDeviceLinux::onCharacter(wchar_t ch)
{
	KeyEvent ke;
	ke.type = KeyEventType::Character;
	ke.character = ch;
	m_keyEvents.push_back(ke);
}

}
