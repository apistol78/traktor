/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Input/GameInput/InputDriverGameInput.h"

#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "Input/GameInput/GamepadDeviceGameInput.h"
#include "Input/GameInput/KeyboardDeviceGameInput.h"
#include "Input/GameInput/MouseDeviceGameInput.h"

namespace traktor::input
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InputDriverGameInput", 0, InputDriverGameInput, IInputDriver)

InputDriverGameInput::~InputDriverGameInput()
{
	destroy();
}

void InputDriverGameInput::destroy()
{
	if (m_deviceCallbackToken != 0)
	{
		m_gameInput->UnregisterCallback(m_deviceCallbackToken, 1000000);
		m_deviceCallbackToken = 0;
	}

	for (auto device : m_devices)
		device->destroy();

	m_keyboardDevice = nullptr;
	m_mouseDevice = nullptr;
	m_gamepadDevices.clear();
	m_devices.clear();
	m_connected.clear();
	m_gameInput.release();
}

bool InputDriverGameInput::create(const SystemApplication& sysapp, const SystemWindow& syswin, InputCategory inputCategories)
{
	const HWND hWnd = (HWND)syswin.hWnd;
	if (!hWnd)
	{
		log::error << L"Unable to create GameInput driver; no window handle." << Endl;
		return false;
	}

	HRESULT hr = GameInputCreate(&m_gameInput.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Unable to create GameInput driver; GameInputCreate failed, hr = " << int32_t(hr) << L"." << Endl;
		return false;
	}

	m_gameInput->SetFocusPolicy(GameInputDisableBackgroundInput);
	m_inputCategories = inputCategories;

	GameInputKind inputKind = GameInputKindUnknown;
	if ((m_inputCategories & InputCategory::Keyboard) != InputCategory::Invalid)
	{
		m_keyboardDevice = new KeyboardDeviceGameInput(m_gameInput, hWnd);
		m_devices.push_back(m_keyboardDevice);
		inputKind |= GameInputKindKeyboard;
	}
	if ((m_inputCategories & InputCategory::Mouse) != InputCategory::Invalid)
	{
		m_mouseDevice = new MouseDeviceGameInput(m_gameInput, hWnd);
		m_devices.push_back(m_mouseDevice);
		inputKind |= GameInputKindMouse;
	}
	if ((m_inputCategories & InputCategory::Joystick) != InputCategory::Invalid)
		inputKind |= GameInputKindGamepad;

	if (inputKind == GameInputKindUnknown)
		return true;

	hr = m_gameInput->RegisterDeviceCallback(
		nullptr,
		inputKind,
		GameInputDeviceConnected,
		GameInputBlockingEnumeration,
		this,
		&InputDriverGameInput::deviceCallback,
		&m_deviceCallbackToken);
	if (FAILED(hr))
	{
		log::error << L"Unable to create GameInput driver; RegisterDeviceCallback failed, hr = " << int32_t(hr) << L"." << Endl;
		return false;
	}

	update();
	return true;
}

int InputDriverGameInput::getDeviceCount()
{
	return int(m_devices.size());
}

Ref< IInputDevice > InputDriverGameInput::getDevice(int index)
{
	return m_devices[index];
}

IInputDriver::UpdateResult InputDriverGameInput::update()
{
	AlignedVector< ComRef< IGameInputDevice > > connected;
	{
		T_ANONYMOUS_VAR(Acquire< CriticalSection >)(m_connectedLock);
		connected.swap(m_connected);
	}

	UpdateResult result = UrOk;
	for (const auto& device : connected)
	{
		const GameInputKind supportedInput = device->GetDeviceInfo()->supportedInput;

		if (m_keyboardDevice && (supportedInput & GameInputKindKeyboard) != 0)
			m_keyboardDevice->addDevice(device);

		if (m_mouseDevice && (supportedInput & GameInputKindMouse) != 0)
			m_mouseDevice->addDevice(device);

		if ((m_inputCategories & InputCategory::Joystick) != InputCategory::Invalid && (supportedInput & GameInputKindGamepad) != 0)
		{
			bool found = false;
			for (auto gamepadDevice : m_gamepadDevices)
				found |= (gamepadDevice->getGameInputDevice() == device);
			if (found)
				continue;

			Ref< GamepadDeviceGameInput > gamepadDevice = new GamepadDeviceGameInput(m_gameInput, device, (int32_t)m_gamepadDevices.size());
			m_gamepadDevices.push_back(gamepadDevice);
			m_devices.push_back(gamepadDevice);

			log::info << L"GameInput gamepad \"" << gamepadDevice->getName() << L"\" connected." << Endl;
			result = UrDevicesChanged;
		}
	}

	return result;
}

void CALLBACK InputDriverGameInput::deviceCallback(
	GameInputCallbackToken callbackToken,
	void* context,
	IGameInputDevice* device,
	uint64_t timestamp,
	GameInputDeviceStatus currentStatus,
	GameInputDeviceStatus previousStatus)
{
	if ((currentStatus & GameInputDeviceConnected) == 0 || (previousStatus & GameInputDeviceConnected) != 0)
		return;

	InputDriverGameInput* this_ = static_cast< InputDriverGameInput* >(context);
	T_ANONYMOUS_VAR(Acquire< CriticalSection >)(this_->m_connectedLock);
	this_->m_connected.push_back(ComRef< IGameInputDevice >(device));
}

}
