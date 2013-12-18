#include "Input/iPhone/InputDriverIPhone.h"
#include "Input/iPhone/InputDeviceTouch.h"
#include "Input/iPhone/InputDeviceTouchMouse.h"

namespace traktor
{
	namespace input
	{
	
class InputDriverIPhoneImpl
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

bool InputDriverIPhoneImpl::create(void* nativeWindowHandle)
{
	UIView* view = (UIView*)nativeWindowHandle;
	CGRect frame = [view frame];
	
	UITouchView* touchView = [[UITouchView alloc] initWithFrame: frame];
	[touchView setCallback: this];
	touchView.multipleTouchEnabled = YES;
	
	[view addSubview: touchView];
	
	m_deviceJoystick = new InputDeviceTouch();
	if (!m_deviceJoystick->create(nativeWindowHandle))
		return false;
		
	m_deviceMouse = new InputDeviceTouchMouse();
	if (!m_deviceMouse->create(nativeWindowHandle))
		return false;
		
	return true;
}

int InputDriverIPhoneImpl::getDeviceCount()
{
	return 2;
}

Ref< IInputDevice > InputDriverIPhoneImpl::getDevice(int index)
{
	if (index == 0)
		return m_deviceJoystick;
	else if (index == 1)
		return m_deviceMouse;
	else
		return 0;
}

void InputDriverIPhoneImpl::touchesBegan(NSSet* touches, UIEvent* event)
{
	m_deviceJoystick->touchesBegan(touches, event);
	m_deviceMouse->touchesBegan(touches, event);
}

void InputDriverIPhoneImpl::touchesMoved(NSSet* touches, UIEvent* event)
{
	m_deviceJoystick->touchesMoved(touches, event);
	m_deviceMouse->touchesMoved(touches, event);
}

void InputDriverIPhoneImpl::touchesEnded(NSSet* touches, UIEvent* event)
{
	m_deviceJoystick->touchesEnded(touches, event);
	m_deviceMouse->touchesEnded(touches, event);
}

void InputDriverIPhoneImpl::touchesCancelled(NSSet* touches, UIEvent* event)
{
	m_deviceJoystick->touchesCancelled(touches, event);
	m_deviceMouse->touchesCancelled(touches, event);
}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InputDriverIPhone", 0, InputDriverIPhone, IInputDriver)

InputDriverIPhone::InputDriverIPhone()
{
}

bool InputDriverIPhone::create(const SystemWindow& systemWindow, uint32_t inputCategories)
{
	m_impl = new InputDriverIPhoneImpl();
	if (!m_impl->create(systemWindow.view))
	{
		m_impl = 0;
		return false;
	}
	return true;
}

int InputDriverIPhone::getDeviceCount()
{
	return m_impl->getDeviceCount();
}

Ref< IInputDevice > InputDriverIPhone::getDevice(int index)
{
	return m_impl->getDevice(index);
}

IInputDriver::UpdateResult InputDriverIPhone::update()
{
	return UrOk;
}

	}
}
