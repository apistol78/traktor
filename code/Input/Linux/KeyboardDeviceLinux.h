/*
 * TRAKTOR
 * Copyright (c) 2024-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/CircularVector.h"
#include "Input/IInputDevice.h"

namespace traktor::input
{

/*! Wayland keyboard device.
 * \ingroup Input
 *
 * Passive state container; the owning InputDriverLinux translates Wayland
 * (xkb) key events into DefaultControl indices and character codepoints and
 * feeds them here.
 */
class KeyboardDeviceLinux : public IInputDevice
{
	T_RTTI_CLASS;

public:
	KeyboardDeviceLinux();

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

	//@{ Called by InputDriverLinux from the Wayland keyboard callbacks.

	void setFocus(bool focus);

	//! Set key state for a DefaultControl index (-1 if unmapped) and queue a Down/Up event.
	void onKey(int32_t control, bool down, bool repeat);

	//! Queue a translated character (text input).
	void onCharacter(wchar_t ch);

	//@}

private:
	bool m_connected;
	bool m_focus;
	CircularVector< KeyEvent, 16 > m_keyEvents;
	uint8_t m_keyStates[256];
};

}
