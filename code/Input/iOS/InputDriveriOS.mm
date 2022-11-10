/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Input/iOS/GamepadDeviceiOS.h"
#include "Input/iOS/InputDriveriOS.h"
#include "Input/iOS/TouchDeviceiOS.h"
#include "Input/iOS/MouseDeviceiOS.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

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
	
class InputDriveriOSImpl
:	public Object
,	public ITouchViewCallback
{
public:
	bool create(void* nativeWindowHandle);

	int getDeviceCount();

	IInputDevice* getDevice(int index);

	IInputDriver::UpdateResult update();

	// ITouchViewCallback

	virtual void touchesBegan(NSSet* touches, UIEvent* event);

	virtual void touchesMoved(NSSet* touches, UIEvent* event);

	virtual void touchesEnded(NSSet* touches, UIEvent* event);

	virtual void touchesCancelled(NSSet* touches, UIEvent* event);

private:
	Ref< TouchDeviceiOS > m_deviceTouch;
	Ref< GamepadDeviceiOS > m_deviceGamepad;
	Ref< MouseDeviceiOS > m_deviceMouse;
	int32_t m_untilCheckOrientation = 0;
	bool m_landscape = false;
};

bool InputDriveriOSImpl::create(void* nativeWindowHandle)
{
	UIView* view = (UIView*)nativeWindowHandle;
	CGRect frame = [view frame];

	m_deviceTouch = new TouchDeviceiOS();
	if (!m_deviceTouch->create(nativeWindowHandle))
		return false;

	m_deviceGamepad = new GamepadDeviceiOS();
	if (!m_deviceGamepad->create(nativeWindowHandle))
		return false;

	m_deviceMouse = new MouseDeviceiOS();
	if (!m_deviceMouse->create(nativeWindowHandle))
		return false;

	UITouchView* touchView = [[UITouchView alloc] initWithFrame: frame];
	[touchView setCallback: this];

	touchView.multipleTouchEnabled = YES;
	touchView.exclusiveTouch = YES;

	[view addSubview: touchView];
	return true;
}

int InputDriveriOSImpl::getDeviceCount()
{
	return 3;
}

IInputDevice* InputDriveriOSImpl::getDevice(int index)
{
	if (index == 0)
		return m_deviceTouch;
	else if (index == 1)
		return m_deviceGamepad;
	else if (index == 2)
		return m_deviceMouse;
	else
		return nullptr;
}

IInputDriver::UpdateResult InputDriveriOSImpl::update()
{
	if (--m_untilCheckOrientation)
	{
		// Check orientation; fairly expensive operation so do not do this too often.
		m_landscape = isLandscape();

		// Propagate to devices.
		m_deviceTouch->setLandscape(m_landscape);
		m_deviceGamepad->setLandscape(m_landscape);
		m_deviceMouse->setLandscape(m_landscape);

		// Assume 60 fps to 0.5 second interval.
		m_untilCheckOrientation = 30;
	}
	return IInputDriver::UrOk;
}

void InputDriveriOSImpl::touchesBegan(NSSet* touches, UIEvent* event)
{
	m_deviceTouch->touchesBegan(touches, event);
	m_deviceGamepad->touchesBegan(touches, event);
	m_deviceMouse->touchesBegan(touches, event);
}

void InputDriveriOSImpl::touchesMoved(NSSet* touches, UIEvent* event)
{
	m_deviceTouch->touchesMoved(touches, event);
	m_deviceGamepad->touchesMoved(touches, event);
	m_deviceMouse->touchesMoved(touches, event);
}

void InputDriveriOSImpl::touchesEnded(NSSet* touches, UIEvent* event)
{
	m_deviceTouch->touchesEnded(touches, event);
	m_deviceGamepad->touchesEnded(touches, event);
	m_deviceMouse->touchesEnded(touches, event);
}

void InputDriveriOSImpl::touchesCancelled(NSSet* touches, UIEvent* event)
{
	m_deviceTouch->touchesCancelled(touches, event);
	m_deviceGamepad->touchesCancelled(touches, event);
	m_deviceMouse->touchesCancelled(touches, event);
}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InputDriveriOS", 0, InputDriveriOS, IInputDriver)

InputDriveriOS::InputDriveriOS()
{
}

bool InputDriveriOS::create(const SystemApplication& sysapp, const SystemWindow& syswin, uint32_t inputCategories)
{
	m_impl = new InputDriveriOSImpl();
	if (!m_impl->create(syswin.view))
	{
		m_impl = 0;
		return false;
	}
	return true;
}

int InputDriveriOS::getDeviceCount()
{
	return m_impl->getDeviceCount();
}

Ref< IInputDevice > InputDriveriOS::getDevice(int index)
{
	return m_impl->getDevice(index);
}

IInputDriver::UpdateResult InputDriveriOS::update()
{
	return m_impl->update();
}

	}
}
