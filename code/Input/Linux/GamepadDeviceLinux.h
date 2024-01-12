/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Input/IInputDevice.h"

namespace traktor::input
{

class GamepadDeviceLinux : public IInputDevice
{
	T_RTTI_CLASS;

public:
	explicit GamepadDeviceLinux(int fd);

	virtual ~GamepadDeviceLinux();

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

private:
	int m_fd;
	int m_buttons[4];
	int m_leftThumbX;
	int m_leftThumbY;
	int m_rightThumbX;
	int m_rightThumbY;
	int m_leftTrigger;
	int m_rightTrigger;
	int m_dpadX;
	int m_dpadY;
};

}
