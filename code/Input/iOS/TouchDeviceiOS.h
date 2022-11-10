/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#import <CoreHaptics/CoreHaptics.h>
#import "Input/iOS/UITouchView.h"

#include <map>
#include "Input/IInputDevice.h"

namespace traktor
{
	namespace input
	{

class TouchDeviceiOS
:	public IInputDevice
,	public ITouchViewCallback
{
	T_RTTI_CLASS;

public:
	TouchDeviceiOS();

	bool create(void* nativeWindowHandle);

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

	// ITouchViewCallback

	virtual void touchesBegan(NSSet* touches, UIEvent* event) override final;

	virtual void touchesMoved(NSSet* touches, UIEvent* event) override final;

	virtual void touchesEnded(NSSet* touches, UIEvent* event) override final;

	virtual void touchesCancelled(NSSet* touches, UIEvent* event) override final;

	// Misc

	void setLandscape(bool landscape) { m_landscape = landscape; }

private:
	bool m_landscape = false;
	float m_width = 0.0f;
	float m_height = 0.0f;
	float m_positionX[3];
	float m_positionY[3];
	int32_t m_fingers = 0;
	CHHapticEngine* m_haptic = nil;
	std::map< const UITouch*, int32_t > m_touch;
};

	}
}

