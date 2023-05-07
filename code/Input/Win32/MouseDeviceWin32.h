/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Input/IInputDevice.h"

namespace traktor::input
{

class MouseDeviceWin32 : public IInputDevice
{
	T_RTTI_CLASS;

public:
	explicit MouseDeviceWin32(HWND hWnd);

	virtual std::wstring getName() const;

	virtual InputCategory getCategory() const;

	virtual bool isConnected() const;

	virtual int32_t getControlCount();

	virtual std::wstring getControlName(int32_t control);

	virtual bool isControlAnalogue(int32_t control) const;

	virtual bool isControlStable(int32_t control) const;

	virtual float getControlValue(int32_t control);

	virtual bool getControlRange(int32_t control, float& outMin, float& outMax) const;

	virtual bool getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const;

	virtual bool getKeyEvent(KeyEvent& outEvent);

	virtual void resetState();

	virtual void readState();

	virtual bool supportRumble() const;

	virtual void setRumble(const InputRumble& rumble);

	virtual void setExclusive(bool exclusive);

private:
	friend class InputDriverWin32;

	bool m_connected;
	HWND m_hWnd;
	POINT m_cursorPosition;
	bool m_haveCursorPosition;
	float m_axisX;
	float m_axisY;
	float m_positionX;
	float m_positionY;
	float m_button1;
	float m_button2;
	float m_button3;
};

}
