/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/keysymdef.h>
#include <X11/extensions/XI.h>
#include <X11/extensions/XInput2.h>
#include <X11/extensions/XKB.h>
#include "Core/Containers/CircularVector.h"
#include "Input/X11/InputDeviceX11.h"

namespace traktor::input
{

class KeyboardDeviceX11 : public InputDeviceX11
{
	T_RTTI_CLASS;

public:
	explicit KeyboardDeviceX11(Display* display, Window window, int deviceId);

	virtual ~KeyboardDeviceX11();

	virtual std::wstring getName() const override final;

	virtual InputCategory getCategory() const override final;

	virtual bool isConnected() const override final;

	virtual int32_t getControlCount() override final;

	virtual std::wstring getControlName(int32_t control) override final;

	virtual bool isControlAnalogue(int32_t control) const override final;

	virtual bool isControlStable(int32_t control) const override final;

	virtual float getControlValue(int32_t control) override final;

	virtual bool getControlRange(int32_t control, float& outMin, float& outMax) const override final;

	virtual bool getDefaultControl(DefaultControl controlType, bool analogue, int32_t& control) const override final;

	virtual bool getKeyEvent(KeyEvent& outEvent) override final;

	virtual void resetState() override final;

	virtual void readState() override final;

	virtual bool supportRumble() const override final;

	virtual void setRumble(const InputRumble& rumble) override final;

	virtual void setExclusive(bool exclusive) override final;

	virtual void consumeEvent(XEvent& evt) override final;

private:
	Display* m_display;
	Window m_window;
	int m_deviceId;
	XkbDescPtr m_kbdesc;
	bool m_connected;
	bool m_exclusive;
	bool m_focus;
	bool m_haveGrab;
	CircularVector< KeyEvent, 16 > m_keyEvents;
	uint8_t m_keyStates[256];

	void setFocus(bool focus);
};

}
