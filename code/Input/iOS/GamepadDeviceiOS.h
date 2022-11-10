/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#import "Input/iOS/UITouchView.h"

#include "Core/Containers/SmallMap.h"
#include "Input/IInputDevice.h"

namespace traktor
{
	namespace input
	{

class GamepadDeviceiOS
:	public IInputDevice
,	public ITouchViewCallback
{
	T_RTTI_CLASS;

public:
	GamepadDeviceiOS();

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

	void setLandscape(bool landscape) {}

private:
	struct IControl
	{
		virtual void begin(UITouch* touch) = 0;

		virtual void end(UITouch* touch) = 0;

		virtual void move(GamepadDeviceiOS* device, UITouch* touch) = 0;
	};

	struct Pad : public IControl
	{
		float axisX;
		float axisY;
		CGPoint origin;

		Pad()
		:	axisX(0.0f)
		,	axisY(0.0f)
		{
		}

		virtual void begin(UITouch* touch);

		virtual void end(UITouch* touch);

		virtual void move(GamepadDeviceiOS* device, UITouch* touch);
	};

	struct Button : public IControl
	{
		float value;

		Button()
		:	value(0.0f)
		{
		}

		virtual void begin(UITouch* touch);

		virtual void end(UITouch* touch);

		virtual void move(GamepadDeviceiOS* device, UITouch* touch);
	};

	CGPoint m_pivot;
	float m_radius;
	Pad m_leftPad;
	Pad m_rightPad;
	Button m_leftButton;
	Button m_rightButton;
	IControl* m_controls[4];
	SmallMap< UITouch*, IControl* > m_track;
};

	}
}

