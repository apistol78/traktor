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
#include "Input/iOS/MouseDeviceiOS.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

const struct MouseControlMap
{
	const wchar_t* name;
	InputDefaultControlType controlType;
	bool analogue;
}
c_mouseControlMap[] =
{
	{ L"Mouse button", DtButton1, false },
	{ L"Mouse X axis", DtPositionX, true },
	{ L"Mouse Y axis", DtPositionY, true }
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.MouseDeviceiOS", MouseDeviceiOS, IInputDevice)

bool MouseDeviceiOS::create(void* nativeWindowHandle)
{
	UIView* view = (UIView*)nativeWindowHandle;
	CGRect frame = [view frame];

	m_width = frame.size.width;
	m_height = frame.size.height;
	m_landscape = isLandscape();

	return true;
}

std::wstring MouseDeviceiOS::getName() const
{
	return L"Touch Mouse";
}

InputCategory MouseDeviceiOS::getCategory() const
{
	return CtMouse;
}

bool MouseDeviceiOS::isConnected() const
{
	return true;
}

int32_t MouseDeviceiOS::getControlCount()
{
	return sizeof_array(c_mouseControlMap);
}

std::wstring MouseDeviceiOS::getControlName(int32_t control)
{
	return c_mouseControlMap[control].name;
}

bool MouseDeviceiOS::isControlAnalogue(int32_t control) const
{
	return c_mouseControlMap[control].analogue;
}

bool MouseDeviceiOS::isControlStable(int32_t control) const
{
	return false;
}

float MouseDeviceiOS::getControlValue(int32_t control)
{
	const MouseControlMap& mc = c_mouseControlMap[control];
	if (mc.controlType == DtPositionX)
		return m_positionX;
	else if (mc.controlType == DtPositionY)
		return m_positionY;
	else if (mc.controlType == DtButton1)
		return m_button ? 1.0f : 0.0f;
	else
		return 0.0f;
}

bool MouseDeviceiOS::getControlRange(int32_t control, float& outMin, float& outMax) const
{
	const MouseControlMap& mc = c_mouseControlMap[control];
	if (mc.controlType == DtPositionX)
	{
		outMin = 0.0f;
		outMax = m_width;
		return true;
	}
	else if (mc.controlType == DtPositionY)
	{
		outMin = 0.0f;
		outMax = m_height;
		return true;
	}
	else
		return false;
}

bool MouseDeviceiOS::getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const
{
	for (int32_t i = 0; i < sizeof_array(c_mouseControlMap); ++i)
	{
		const MouseControlMap& mc = c_mouseControlMap[i];
		if (mc.controlType == controlType && mc.analogue == analogue)
		{
			control = i;
			return true;
		}
	}
	return false;
}

bool MouseDeviceiOS::getKeyEvent(KeyEvent& outEvent)
{
	return false;
}

void MouseDeviceiOS::resetState()
{
	m_positionX = 0.0f;
	m_positionY = 0.0f;
	m_button = false;
}

void MouseDeviceiOS::readState()
{
}

bool MouseDeviceiOS::supportRumble() const
{
	return false;
}

void MouseDeviceiOS::setRumble(const InputRumble& rumble)
{
}

void MouseDeviceiOS::setExclusive(bool exclusive)
{
}

void MouseDeviceiOS::touchesBegan(NSSet* touches, UIEvent* event)
{
	if (m_touch != nullptr)
		return;

	m_touch = touches.anyObject;
	
	CGPoint location = [m_touch locationInView: nil];
	if (!m_landscape)
	{
		m_positionX = location.x;
		m_positionY = location.y;
	}
	else
	{
		m_positionX = m_height - location.y;
		m_positionY = location.x;
	}
	m_button = true;
}

void MouseDeviceiOS::touchesMoved(NSSet* touches, UIEvent* event)
{
	if (m_touch != nullptr && [touches containsObject: m_touch] == YES)
	{
		CGPoint location = [m_touch locationInView: nil];
		if (!m_landscape)
		{
			m_positionX = location.x;
			m_positionY = location.y;
		}
		else
		{
			m_positionX = m_height - location.y;
			m_positionY = location.x;
		}
	}
}

void MouseDeviceiOS::touchesEnded(NSSet* touches, UIEvent* event)
{
	if (m_touch != nullptr && [touches containsObject: m_touch] == YES)
	{	
		CGPoint location = [m_touch locationInView: nil];
		if (!m_landscape)
		{
			m_positionX = location.x;
			m_positionY = location.y;
		}
		else
		{
			m_positionX = m_height - location.y;
			m_positionY = location.x;
		}
		m_button = false;
		m_touch = nullptr;
	}
}

void MouseDeviceiOS::touchesCancelled(NSSet* touches, UIEvent* event)
{
	m_button = false;
	m_touch = nullptr;
}

	}
}
