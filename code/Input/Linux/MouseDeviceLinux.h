/*
 * TRAKTOR
 * Copyright (c) 2024-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Input/IInputDevice.h"

namespace traktor::input
{

/*! Wayland mouse device.
 * \ingroup Input
 *
 * Passive state container; the owning InputDriverLinux feeds it pointer
 * events received from the Wayland seat. In non-exclusive mode the device
 * reports absolute position plus per-frame motion deltas; in exclusive
 * mode the driver locks the pointer and feeds relative motion deltas.
 */
class MouseDeviceLinux : public IInputDevice
{
	T_RTTI_CLASS;

public:
	explicit MouseDeviceLinux(int32_t width, int32_t height);

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

	//@{ Called by InputDriverLinux from the Wayland seat callbacks.

	void setFocus(bool focus);

	void setSize(int32_t width, int32_t height);

	//! Absolute pointer position (window-relative, logical pixels).
	void onPointerEnter(double x, double y);

	void onPointerMotion(double x, double y);

	//! Relative motion delta from the relative-pointer protocol (exclusive mode).
	void onRelativeMotion(double dx, double dy);

	void onButton(uint32_t button, bool down);

	void onWheel(double value);

	//@}

	bool isExclusive() const { return m_exclusive; }

	bool haveFocus() const { return m_focus; }

private:
	bool m_connected;
	bool m_exclusive;
	bool m_focus;
	int32_t m_raw[2];		//!< Accumulated X/Y motion delta since last readState.
	double m_wheel;			//!< Accumulated vertical scroll since last readState.
	float m_axis[3];		//!< X/Y/Z axis output (Z = wheel).
	int32_t m_position[2];
	double m_last[2];
	bool m_haveLast;
	float m_button[3];		//!< Left/Right/Middle button states.
	int32_t m_width;
	int32_t m_height;
};

}
