#include "Core/Log/Log.h"
#include "Input/iOS/InputDriveriOS.h"
#include "Input/iOS/InputDeviceTouch.h"
#include "Input/iOS/InputDeviceTouchMouse.h"

namespace traktor
{
	namespace input
	{
	
class InputDriveriOSImpl
:	public Object
,	public ITouchViewCallback
{
public:
	bool create(void* nativeWindowHandle);
	
	int getDeviceCount();
	
	Ref< IInputDevice > getDevice(int index);

	// ITouchViewCallback
	
	virtual void touchesBegan(NSSet* touches, UIEvent* event);

	virtual void touchesMoved(NSSet* touches, UIEvent* event);

	virtual void touchesEnded(NSSet* touches, UIEvent* event);

	virtual void touchesCancelled(NSSet* touches, UIEvent* event);

private:
	Ref< InputDeviceTouch > m_deviceJoystick;
	Ref< InputDeviceTouchMouse > m_deviceMouse;
};

bool InputDriveriOSImpl::create(void* nativeWindowHandle)
{
	UIView* view = (UIView*)nativeWindowHandle;
	CGRect frame = [view frame];

	m_deviceJoystick = new InputDeviceTouch();
	if (!m_deviceJoystick->create(nativeWindowHandle))
		return false;
		
	m_deviceMouse = new InputDeviceTouchMouse();
	if (!m_deviceMouse->create(nativeWindowHandle))
		return false;

	UITouchView* touchView = [[UITouchView alloc] initWithFrame: frame];
	[touchView setCallback: this];

	[view addSubview: touchView];
	return true;
}

int InputDriveriOSImpl::getDeviceCount()
{
	return 2;
}

Ref< IInputDevice > InputDriveriOSImpl::getDevice(int index)
{
	if (index == 0)
		return m_deviceJoystick;
	else if (index == 1)
		return m_deviceMouse;
	else
		return 0;
}

void InputDriveriOSImpl::touchesBegan(NSSet* touches, UIEvent* event)
{
	m_deviceJoystick->touchesBegan(touches, event);
	m_deviceMouse->touchesBegan(touches, event);
}

void InputDriveriOSImpl::touchesMoved(NSSet* touches, UIEvent* event)
{
	m_deviceJoystick->touchesMoved(touches, event);
	m_deviceMouse->touchesMoved(touches, event);
}

void InputDriveriOSImpl::touchesEnded(NSSet* touches, UIEvent* event)
{
	m_deviceJoystick->touchesEnded(touches, event);
	m_deviceMouse->touchesEnded(touches, event);
}

void InputDriveriOSImpl::touchesCancelled(NSSet* touches, UIEvent* event)
{
	m_deviceJoystick->touchesCancelled(touches, event);
	m_deviceMouse->touchesCancelled(touches, event);
}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InputDriveriOS", 0, InputDriveriOS, IInputDriver)

InputDriveriOS::InputDriveriOS()
{
}

bool InputDriveriOS::create(const SystemWindow& systemWindow, uint32_t inputCategories)
{
	m_impl = new InputDriveriOSImpl();
	if (!m_impl->create(systemWindow.view))
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
	return UrOk;
}

	}
}
