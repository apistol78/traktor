/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#import <UIKit/UIKit.h>

#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Input/iOS/GamepadDeviceiOS.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.GamepadDeviceiOS", GamepadDeviceiOS, IInputDevice)

GamepadDeviceiOS::GamepadDeviceiOS()
:	m_radius(0.0f)
{
}

bool GamepadDeviceiOS::create(void* nativeWindowHandle)
{
	UIView* view = (UIView*)nativeWindowHandle;
	CGRect frame = [view frame];

	float cx = frame.origin.x + frame.size.width / 2.0f;
	float cy = frame.origin.y + frame.size.height / 2.0f;

	float slabW = frame.size.width / 2.0f;
	float slabH = frame.size.height / 2.0f;
	m_radius = std::min(slabW, slabH) / 2.0f;

	m_pivot = CGPointMake(cx, cy);

	m_controls[0] = &m_leftButton;
	m_controls[1] = &m_rightButton;
	m_controls[2] = &m_rightPad;
	m_controls[3] = &m_leftPad;
	return true;
}

std::wstring GamepadDeviceiOS::getName() const
{
	return L"Gamepad";
}

InputCategory GamepadDeviceiOS::getCategory() const
{
	return CtJoystick;
}

bool GamepadDeviceiOS::isConnected() const
{
	return true;
}

int32_t GamepadDeviceiOS::getControlCount()
{
	return 0;
}

std::wstring GamepadDeviceiOS::getControlName(int32_t control)
{
	return L"";
}

bool GamepadDeviceiOS::isControlAnalogue(int32_t control) const
{
	return true;
}

bool GamepadDeviceiOS::isControlStable(int32_t control) const
{
	return true;
}

float GamepadDeviceiOS::getControlValue(int32_t control)
{
	if (control == -1)
		return m_leftPad.axisX;
	else if (control == -2)
		return m_leftPad.axisY;
	else if (control == -3)
		return m_rightPad.axisX;
	else if (control == -4)
		return m_rightPad.axisY;
	else if (control == -5)
		return m_leftButton.value;
	else if (control == -6)
		return m_rightButton.value;
	else if (control == -7)
		return m_leftPad.axisY > 0.5f ? 1.0f : 0.0f;
	else if (control == -8)
		return m_leftPad.axisY < -0.5f ? 1.0f : 0.0f;
	else
		return 0.0f;
}

bool GamepadDeviceiOS::getControlRange(int32_t control, float& outMin, float& outMax) const
{
	return false;
}

bool GamepadDeviceiOS::getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const
{
	control = 0;

	switch (controlType)
	{
	case DtThumbLeftX:
		if (analogue)
			control = -1;
		break;

	case DtThumbLeftY:
		if (analogue)
			control = -2;
		break;

	case DtThumbRightX:
		if (analogue)
			control = -3;
		break;

	case DtThumbRightY:
		if (analogue)
			control = -4;
		break;

	case DtButton1:
		if (!analogue)
			control = -5;
		break;

	case DtButton2:
		if (!analogue)
			control = -6;
		break;

	case DtUp:
		if (!analogue)
			control = -7;
		break;

	case DtDown:
		if (!analogue)
			control = -8;
		break;

	default:
		return false;
	}

	return control != 0;
}

bool GamepadDeviceiOS::getKeyEvent(KeyEvent& outEvent)
{
	return false;
}

void GamepadDeviceiOS::resetState()
{
}

void GamepadDeviceiOS::readState()
{
}

bool GamepadDeviceiOS::supportRumble() const
{
	return false;
}

void GamepadDeviceiOS::setRumble(const InputRumble& rumble)
{
}

void GamepadDeviceiOS::setExclusive(bool exclusive)
{
}

void GamepadDeviceiOS::touchesBegan(NSSet* touches, UIEvent* event)
{
	for (UITouch* touch in touches)
	{
		CGPoint location = [touch locationInView: nil];
		IControl* control = 0;

		if (location.y < m_pivot.y)
		{
			if (location.x < m_pivot.x)
				control = m_controls[0];
			else
				control = m_controls[1];
		}
		else
		{
			if (location.x < m_pivot.x)
				control = m_controls[3];
			else
				control = m_controls[2];
		}

		if (control)
		{
			control->begin(touch);
			m_track[touch] = control;
		}
	}
}

void GamepadDeviceiOS::touchesMoved(NSSet* touches, UIEvent* event)
{
	for (UITouch* touch in touches)
	{
		IControl* control = m_track[touch];
		if (control)
			control->move(this, touch);
	}
}

void GamepadDeviceiOS::touchesEnded(NSSet* touches, UIEvent* event)
{
	for (UITouch* touch in touches)
	{
		IControl* control = m_track[touch];
		if (control)
			control->end(touch);
	}
}

void GamepadDeviceiOS::touchesCancelled(NSSet* touches, UIEvent* event)
{
	for (UITouch* touch in touches)
	{
		IControl* control = m_track[touch];
		if (control)
			control->end(touch);
	}
}

void GamepadDeviceiOS::Pad::begin(UITouch* touch)
{
	origin = [touch locationInView: nil];
}

void GamepadDeviceiOS::Pad::end(UITouch* touch)
{
	axisX =
	axisY = 0.0f;
}

void GamepadDeviceiOS::Pad::move(GamepadDeviceiOS* device, UITouch* touch)
{
	CGPoint location = [touch locationInView: nil];

	float offsetX = location.x - origin.x;
	float offsetY = location.y - origin.y;

	const float c_deadZone = 8.0f;

	if (abs(offsetX) > c_deadZone)
		offsetX = sign(offsetX) * (abs(offsetX) - c_deadZone);
	else
		offsetX = 0.0f;

	if (abs(offsetY) > c_deadZone)
		offsetY = sign(offsetY) * (abs(offsetY) - c_deadZone);
	else
		offsetY = 0.0f;

	axisX =  clamp(offsetX / device->m_radius, -1.0f, 1.0f);
	axisY = -clamp(offsetY / device->m_radius, -1.0f, 1.0f);
}

void GamepadDeviceiOS::Button::begin(UITouch* touch)
{
	value = 1.0f;
}

void GamepadDeviceiOS::Button::end(UITouch* touch)
{
	value = 0.0f;
}

void GamepadDeviceiOS::Button::move(GamepadDeviceiOS* device, UITouch* touch)
{
}

	}
}
