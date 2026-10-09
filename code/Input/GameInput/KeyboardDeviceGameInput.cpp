/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Input/GameInput/KeyboardDeviceGameInput.h"

#include "Core/Log/Log.h"

#include <commctrl.h>

namespace traktor::input
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.KeyboardDeviceGameInput", KeyboardDeviceGameInput, IInputDevice)

KeyboardDeviceGameInput::KeyboardDeviceGameInput(IGameInput* gameInput, HWND hWnd)
	: m_gameInput(gameInput)
	, m_hWnd(hWnd)
{
	m_subclassed = (SetWindowSubclass(m_hWnd, &KeyboardDeviceGameInput::wndProc, (UINT_PTR)this, (DWORD_PTR)this) != FALSE);
	if (!m_subclassed)
		log::warning << L"Unable to subclass window; no keyboard events available." << Endl;

	resetState();
}

void KeyboardDeviceGameInput::destroy()
{
	if (m_subclassed)
	{
		RemoveWindowSubclass(m_hWnd, &KeyboardDeviceGameInput::wndProc, (UINT_PTR)this);
		m_subclassed = false;
	}
	m_devices.clear();
	m_connected = false;
}

std::wstring KeyboardDeviceGameInput::getName() const
{
	return L"Keyboard";
}

InputCategory KeyboardDeviceGameInput::getCategory() const
{
	return InputCategory::Keyboard;
}

bool KeyboardDeviceGameInput::isConnected() const
{
	return m_connected;
}

int32_t KeyboardDeviceGameInput::getControlCount()
{
	return sizeof_array(c_scanCodeControlKeys);
}

std::wstring KeyboardDeviceGameInput::getControlName(int32_t control)
{
	const uint32_t code = c_scanCodeControlKeys[control];
	if (code == 0)
		return L"";

	LONG keyLParam = LONG(code & 0x7f) << 16;
	if (code == 0xc5)
		keyLParam = 0x45 << 16;
	else if (code == 0x45 || (code & 0x80) != 0)
		keyLParam |= 1 << 24;

	wchar_t keyName[64];
	if (GetKeyNameText(keyLParam, keyName, sizeof_array(keyName)) != 0)
		return keyName;
	else
		return L"";
}

bool KeyboardDeviceGameInput::isControlAnalogue(int32_t control) const
{
	return false;
}

bool KeyboardDeviceGameInput::isControlStable(int32_t control) const
{
	return false;
}

float KeyboardDeviceGameInput::getControlValue(int32_t control)
{
	return m_state[control] ? 1.0f : 0.0f;
}

bool KeyboardDeviceGameInput::getControlRange(int32_t control, float& outMin, float& outMax) const
{
	outMin = 0.0f;
	outMax = 1.0f;
	return true;
}

bool KeyboardDeviceGameInput::getDefaultControl(DefaultControl controlType, bool analogue, int32_t& control) const
{
	if (analogue)
		return false;

	control = (int32_t)controlType;
	return control >= 0 && control < (int32_t)sizeof_array(c_scanCodeControlKeys) && c_scanCodeControlKeys[control] != 0;
}

bool KeyboardDeviceGameInput::getKeyEvent(KeyEvent& outEvent)
{
	if (m_keyEvents.empty())
		return false;

	outEvent = m_keyEvents.front();
	m_keyEvents.pop_front();
	return true;
}

void KeyboardDeviceGameInput::resetState()
{
	std::memset(m_state, 0, sizeof(m_state));
	m_keyEvents.clear();
}

void KeyboardDeviceGameInput::readState()
{
	std::memset(m_state, 0, sizeof(m_state));

	const bool connected = (GetForegroundWindow() == GetAncestor(m_hWnd, GA_ROOT));
	if (connected && !m_connected)
		m_connectedTimestamp = m_gameInput->GetCurrentTimestamp();
	m_connected = connected;
	if (!m_connected)
		return;

	for (const auto& device : m_devices)
	{
		ComRef< IGameInputReading > reading;
		if (FAILED(m_gameInput->GetCurrentReading(GameInputKindKeyboard, device, &reading.getAssign())))
			continue;
		if (reading->GetTimestamp() < m_connectedTimestamp)
			continue;

		GameInputKeyState keyStates[32];
		const uint32_t keyCount = reading->GetKeyState(sizeof_array(keyStates), keyStates);
		for (uint32_t i = 0; i < keyCount; ++i)
		{
			const GameInputKeyState& keyState = keyStates[i];
			const uint32_t control = translateFromScanCode(keyState.scanCode, (keyState.scanCode & 0xff80) != 0, keyState.virtualKey);
			if (control != 0)
				m_state[control] = true;
		}
	}
}

bool KeyboardDeviceGameInput::supportRumble() const
{
	return false;
}

void KeyboardDeviceGameInput::setRumble(const InputRumble& rumble)
{
}

void KeyboardDeviceGameInput::setExclusive(bool exclusive)
{
}

void KeyboardDeviceGameInput::addDevice(IGameInputDevice* device)
{
	for (const auto& existing : m_devices)
		if (existing == device)
			return;
	m_devices.push_back(ComRef< IGameInputDevice >(device));
}

LRESULT CALLBACK KeyboardDeviceGameInput::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	KeyboardDeviceGameInput* this_ = reinterpret_cast< KeyboardDeviceGameInput* >(dwRefData);

	if (uMsg == WM_CHAR)
	{
		if (wParam != '\r')
		{
			const KeyEvent ke = {
				.type = KeyEventType::Character,
				.character = (wchar_t)wParam
			};
			this_->m_keyEvents.push_back(ke);
		}
	}
	else if (uMsg == WM_KEYDOWN || uMsg == WM_KEYUP)
	{
		const uint32_t keyCode = translateFromScanCode(uint32_t(lParam >> 16) & 0xff, (lParam & (1 << 24)) != 0, uint32_t(wParam));
		if (keyCode != 0)
		{
			const KeyEvent ke = {
				.type = (uMsg == WM_KEYDOWN) ? KeyEventType::Down : KeyEventType::Up,
				.keyCode = keyCode
			};
			this_->m_keyEvents.push_back(ke);
		}
	}

	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

}
