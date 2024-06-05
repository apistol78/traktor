/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include "Core/RefArray.h"
#include "Core/Misc/ComRef.h"
#include "Input/IInputDriver.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_DI8_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

struct SystemWindow;

}

namespace traktor::input
{

class T_DLLCLASS InputDriverDi8 : public IInputDriver
{
	T_RTTI_CLASS;

public:
	virtual ~InputDriverDi8();

	void destroy();

	virtual bool create(const SystemApplication& sysapp, const SystemWindow& syswin, InputCategory inputCategories) override final;

	virtual int getDeviceCount() override final;

	virtual Ref< IInputDevice > getDevice(int index) override final;

	virtual UpdateResult update() override final;

private:
	static BOOL CALLBACK enumDevicesCallback(const DIDEVICEINSTANCE* deviceInstance, VOID* context);

	bool addDevice(const DIDEVICEINSTANCE* instance);

	HWND m_hWnd = NULL;
	InputCategory m_inputCategories = InputCategory::Invalid;
	RefArray< IInputDevice > m_devices;
	ComRef< IDirectInput8 > m_directInput;
};

}
