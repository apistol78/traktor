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
#include "Input/iOS/TouchDeviceiOS.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

const struct TouchControlMap
{
	const wchar_t* name;
	InputDefaultControlType controlType;
	bool analogue;
}
c_touchControlMap[] =
{
	{ L"Finger 1", DtButton1, false },
	{ L"Finger 2", DtButton2, false },
	{ L"Finger 3", DtButton3, false },
	{ L"Touch X axis 1", DtPositionX, true },
	{ L"Touch Y axis 1", DtPositionY, true },
	{ L"Touch X axis 2", DtPositionX2, true },
	{ L"Touch Y axis 2", DtPositionY2, true },
	{ L"Touch X axis 3", DtPositionX3, true },
	{ L"Touch Y axis 3", DtPositionY3, true }
};

bool isLandscape()
{
	UIWindow* firstWindow = [[[UIApplication sharedApplication] windows] firstObject];
	if (firstWindow == nil)
		return false;

	UIWindowScene* windowScene = firstWindow.windowScene;
	if (windowScene == nil)
		return false;

	return UIInterfaceOrientationIsLandscape(windowScene.interfaceOrientation);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.TouchDeviceiOS", TouchDeviceiOS, IInputDevice)

TouchDeviceiOS::TouchDeviceiOS()
:	m_landscape(false)
,	m_width(0.0f)
,	m_height(0.0f)
{
	resetState();
}

bool TouchDeviceiOS::create(void* nativeWindowHandle)
{
	UIView* view = (UIView*)nativeWindowHandle;
	CGRect frame = [view frame];

	m_width = frame.size.width;
	m_height = frame.size.height;

	// Check screen orientation.
	m_landscape = isLandscape();

	// Initialize haptic engine, if supported by device.
	if (CHHapticEngine.capabilitiesForHardware.supportsHaptics)
	{
		NSError* error = nil;
		m_haptic = [[CHHapticEngine alloc] initAndReturnError: &error];
		if (error != nil)
		{
			log::warning << L"Haptic supported but failed to initialize; haptic feedback disabled." << Endl;
			m_haptic = nil;
		}
	}

	return true;
}

std::wstring TouchDeviceiOS::getName() const
{
	return L"Touch";
}

InputCategory TouchDeviceiOS::getCategory() const
{
	return CtTouch;
}

bool TouchDeviceiOS::isConnected() const
{
	return true;
}

int32_t TouchDeviceiOS::getControlCount()
{
	return sizeof_array(c_touchControlMap);
}

std::wstring TouchDeviceiOS::getControlName(int32_t control)
{
	return c_touchControlMap[control].name;
}

bool TouchDeviceiOS::isControlAnalogue(int32_t control) const
{
	return c_touchControlMap[control].analogue;
}

bool TouchDeviceiOS::isControlStable(int32_t control) const
{
	return false;
}

float TouchDeviceiOS::getControlValue(int32_t control)
{
	const TouchControlMap& mc = c_touchControlMap[control];
	if (mc.controlType == DtPositionX)
		return m_positionX[0];
	else if (mc.controlType == DtPositionY)
		return m_positionY[0];
	else if (mc.controlType == DtPositionX2)
		return m_positionX[1];
	else if (mc.controlType == DtPositionY2)
		return m_positionY[1];
	else if (mc.controlType == DtPositionX3)
		return m_positionX[2];
	else if (mc.controlType == DtPositionY3)
		return m_positionY[2];
	else if (mc.controlType == DtButton1)
		return (m_fingers == 1) ? 1.0f : 0.0f;
	else if (mc.controlType == DtButton2)
		return (m_fingers == 2) ? 1.0f : 0.0f;
	else if (mc.controlType == DtButton3)
		return (m_fingers == 3) ? 1.0f : 0.0f;
	else
		return 0.0f;
}

bool TouchDeviceiOS::getControlRange(int32_t control, float& outMin, float& outMax) const
{
	const TouchControlMap& tc = c_touchControlMap[control];
	if (tc.controlType == DtPositionX || tc.controlType == DtPositionX2 || tc.controlType == DtPositionX3)
	{
		outMin = 0.0f;
		outMax = m_width;
		return true;
	}
	else if (tc.controlType == DtPositionY || tc.controlType == DtPositionY2 || tc.controlType == DtPositionY3)
	{
		outMin = 0.0f;
		outMax = m_height;
		return true;
	}
	else
		return false;
}

bool TouchDeviceiOS::getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const
{
	for (int32_t i = 0; i < sizeof_array(c_touchControlMap); ++i)
	{
		const TouchControlMap& tc = c_touchControlMap[i];
		if (tc.controlType == controlType && tc.analogue == analogue)
		{
			control = i;
			return true;
		}
	}
	return false;
}

bool TouchDeviceiOS::getKeyEvent(KeyEvent& outEvent)
{
	return false;
}

void TouchDeviceiOS::resetState()
{
	for (int32_t i = 0; i < 3; ++i)
	{
		m_positionX[i] = 0.0f;
		m_positionY[i] = 0.0f;
	}
	m_fingers = 0;
	m_touch.clear();
}

void TouchDeviceiOS::readState()
{
}

bool TouchDeviceiOS::supportRumble() const
{
	return m_haptic != nil;
}

void TouchDeviceiOS::setRumble(const InputRumble& rumble)
{
	// if (rumble.lowFrequencyRumble <= 0.0f && rumble.highFrequencyRumble <= 0)
	// {
	// 	return;
	// }
}

void TouchDeviceiOS::setExclusive(bool exclusive)
{
}

void TouchDeviceiOS::touchesBegan(NSSet* touches, UIEvent* event)
{
	m_fingers = 0;

	for (UITouch* touch in [event allTouches])
	{
		CGPoint location = [touch locationInView: nil];
		m_touch[touch] = m_fingers;
		if (!m_landscape)
		{
			m_positionX[m_fingers] = location.x;
			m_positionY[m_fingers] = location.y;
		}
		else
		{
			m_positionX[m_fingers] = m_height - location.y;
			m_positionY[m_fingers] = location.x;
		}
		if (++m_fingers >= 3)
			break;
	}
}

void TouchDeviceiOS::touchesMoved(NSSet* touches, UIEvent* event)
{
	for (UITouch* touch in [event allTouches])
	{
		CGPoint location = [touch locationInView: nil];
		int32_t index = -1;

		std::map< const UITouch*, int32_t >::const_iterator i = m_touch.find(touch);
		if (i == m_touch.end())
		{
			if (m_fingers >= 3)
				continue;

			index = m_fingers;
			m_touch[touch] = m_fingers++;
		}
		else
			index = i->second;

		T_ASSERT (index >= 0 && index < m_fingers);

		if (!m_landscape)
		{
			m_positionX[index] = location.x;
			m_positionY[index] = location.y;
		}
		else
		{
			m_positionX[index] = m_height - location.y;
			m_positionY[index] = location.x;
		}
	}
}

void TouchDeviceiOS::touchesEnded(NSSet* touches, UIEvent* event)
{
	for (UITouch* touch in [event allTouches])
	{
		CGPoint location = [touch locationInView: nil];

		std::map< const UITouch*, int32_t >::const_iterator i = m_touch.find(touch);
		if (i == m_touch.end())
			continue;

		int32_t index = i->second;
		if (!m_landscape)
		{
			m_positionX[index] = location.x;
			m_positionY[index] = location.y;
		}
		else
		{
			m_positionX[index] = m_height - location.y;
			m_positionY[index] = location.x;
		}
	}

	m_touch.clear();
	m_fingers = 0;
}

void TouchDeviceiOS::touchesCancelled(NSSet* touches, UIEvent* event)
{
}

	}
}
