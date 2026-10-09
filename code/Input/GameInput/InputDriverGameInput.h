/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Misc/ComRef.h"
#include "Core/RefArray.h"
#include "Core/Thread/CriticalSection.h"
#include "Input/IInputDriver.h"

#include <GameInput.h>
#include <windows.h>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_GAMEINPUT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::input
{

class GamepadDeviceGameInput;
class KeyboardDeviceGameInput;
class MouseDeviceGameInput;

/*! GameInput driver.
 * \ingroup Input
 */
class T_DLLCLASS InputDriverGameInput : public IInputDriver
{
	T_RTTI_CLASS;

public:
	virtual ~InputDriverGameInput();

	virtual void destroy() override final;

	virtual bool create(const SystemApplication& sysapp, const SystemWindow& syswin, InputCategory inputCategories) override final;

	virtual int getDeviceCount() override final;

	virtual Ref< IInputDevice > getDevice(int index) override final;

	virtual UpdateResult update() override final;

private:
	InputCategory m_inputCategories = InputCategory::Invalid;
	ComRef< IGameInput > m_gameInput;
	GameInputCallbackToken m_deviceCallbackToken = 0;
	CriticalSection m_connectedLock;
	AlignedVector< ComRef< IGameInputDevice > > m_connected;
	Ref< KeyboardDeviceGameInput > m_keyboardDevice;
	Ref< MouseDeviceGameInput > m_mouseDevice;
	RefArray< GamepadDeviceGameInput > m_gamepadDevices;
	RefArray< IInputDevice > m_devices;

	static void CALLBACK deviceCallback(
		GameInputCallbackToken callbackToken,
		void* context,
		IGameInputDevice* device,
		uint64_t timestamp,
		GameInputDeviceStatus currentStatus,
		GameInputDeviceStatus previousStatus);
};

}
