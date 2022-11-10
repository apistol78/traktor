/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Platform.h"
#include "Core/Containers/StaticVector.h"
#include "Input/IInputDevice.h"

struct AInputEvent;

namespace traktor
{
	namespace input
	{

class TouchDeviceAndroid : public IInputDevice
{
	T_RTTI_CLASS;

public:
	TouchDeviceAndroid(const SystemWindow& syswin);

	virtual std::wstring getName() const override final;

	virtual InputCategory getCategory() const override final;

	virtual bool isConnected() const override final;

	virtual int32_t getControlCount() override final;

	virtual std::wstring getControlName(int32_t control) override final;

	virtual bool isControlAnalogue(int32_t control) const override final;

	virtual bool isControlStable(int32_t control) const override final;

	virtual float getControlValue(int32_t control) override final;

	virtual bool getControlRange(int32_t control, float& outMin, float& outMax) const override final;

	virtual bool getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const override final;

	virtual bool getKeyEvent(KeyEvent& outEvent) override final;

	virtual void resetState() override final;

	virtual void readState() override final;

	virtual bool supportRumble() const override final;

	virtual void setRumble(const InputRumble& rumble) override final;

	virtual void setExclusive(bool exclusive) override final;

private:
	friend class InputDriverAndroid;

	struct ActivePointer
	{
		int32_t id;
		bool active;
	};

	int32_t m_width;
	int32_t m_height;
	float m_positionX[4];
	float m_positionY[4];
	StaticVector< ActivePointer, 4 > m_activePointerIds;

	void handleInput(AInputEvent* event);
};

	}
}

