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
#include "Input/IInputDevice.h"
#include "Core/Misc/ComRef.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_DI8_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::input
{

class T_DLLCLASS JoystickDeviceDi8 : public IInputDevice
{
	T_RTTI_CLASS;

public:
	explicit JoystickDeviceDi8(HWND hWnd, const ComRef< IDirectInputDevice8 >& diDevice, const DIDEVICEINSTANCE* deviceInstance);

	virtual std::wstring getName() const;

	virtual InputCategory getCategory() const;

	virtual bool isConnected() const;

	virtual int32_t getControlCount();

	virtual std::wstring getControlName(int32_t control);

	virtual bool isControlAnalogue(int32_t control) const;

	virtual bool isControlStable(int32_t control) const;

	virtual float getControlValue(int32_t control);

	virtual bool getControlRange(int32_t control, float& outMin, float& outMax) const;

	virtual bool getDefaultControl(DefaultControl controlType, bool analogue, int32_t& control) const;

	virtual bool getKeyEvent(KeyEvent& outEvent);

	virtual void resetState();

	virtual void readState();

	virtual bool supportRumble() const;

	virtual void setRumble(const InputRumble& rumble);

	virtual void setExclusive(bool exclusive);

private:
	struct ControlInfo
	{
		std::wstring name;
		DefaultControl controlType;
		uint32_t offset;
		bool analogue;
		bool inverted;
	};

	HWND m_hWnd;
	ComRef< IDirectInputDevice8 > m_device;
	std::wstring m_name;
	DIJOYSTATE2 m_state;
	bool m_connected;
	std::vector< ControlInfo > m_controlInfo;

	void collectControls(IDirectInputDevice8* device);
};

}
