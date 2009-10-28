#import <UIKit/UIKit.h>

#include "Input/iPhone/InputDeviceTouch.h"
#include "Core/Math/MathUtils.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputDeviceTouch", InputDeviceTouch, IInputDevice)

InputDeviceTouch::InputDeviceTouch()
:	m_leftAxisX(0.0f)
,	m_leftAxisY(0.0f)
{
}

bool InputDeviceTouch::create(void* nativeWindowHandle)
{
	UIView* view = (UIView*)nativeWindowHandle;
	CGRect frame = [view frame];
	
	UITouchView* touchView = [[UITouchView alloc] initWithFrame: frame];
	[touchView setCallback: this];
	
	[view addSubview: touchView];
	
	log::info << L"Touch input device created successfully" << Endl;
	return true;
}

std::wstring InputDeviceTouch::getName() const
{
	return L"Touch";
}

InputCategory InputDeviceTouch::getCategory() const
{
	return CtJoystick;
}

bool InputDeviceTouch::isConnected() const
{
	return true;
}

int InputDeviceTouch::getControlCount()
{
	return 0;
}

std::wstring InputDeviceTouch::getControlName(int control)
{
	return L"";
}

bool InputDeviceTouch::isControlAnalogue(int control) const
{
	return true;
}

float InputDeviceTouch::getControlValue(int control)
{
	if (control == -1)
		return m_leftAxisX;
	else if (control == -2)
		return m_leftAxisY;
	else
		return 0.0f;
}

bool InputDeviceTouch::getDefaultControl(InputDefaultControlType controlType, int& control) const
{
	switch (controlType)
	{
	case DtThumbLeftX:
		control = -1;
		break;
		
	case DtThumbLeftY:
		control = -2;
		break;
		
	default:
		return false;
	}
	return true;
}

void InputDeviceTouch::resetState()
{
}

void InputDeviceTouch::readState()
{
}

bool InputDeviceTouch::supportRumble() const
{
	return false;
}

void InputDeviceTouch::setRumble(const InputRumble& rumble)
{
}

void InputDeviceTouch::touchesBegan(NSSet* touches, UIEvent* event)
{
	NSEnumerator* enumerator = [touches objectEnumerator];
	id value;
	
	while ((value = [enumerator nextObject]) != nil)
	{
		UITouch* touch = (UITouch*)value;
		m_firstLocation = [touch locationInView: nil];
		break;
	}
	
	m_leftAxisX = 0.0f;
	m_leftAxisY = 0.0f;
}

void InputDeviceTouch::touchesMoved(NSSet* touches, UIEvent* event)
{
	NSEnumerator* enumerator = [touches objectEnumerator];
	id value;
	
	while ((value = [enumerator nextObject]) != nil)
	{
		UITouch* touch = (UITouch*)value;
		CGPoint currentLocation = [touch locationInView: nil];
		
		float offsetX = currentLocation.x - m_firstLocation.x;
		float offsetY = currentLocation.y - m_firstLocation.y;
		
		m_leftAxisX = clamp(offsetX / 4.0f, -1.0f, 1.0f);
		m_leftAxisY = clamp(offsetY / 4.0f, -1.0f, 1.0f);
		
		log::info << m_leftAxisX << L" " << m_leftAxisY << Endl;
	}
}

void InputDeviceTouch::touchesEnded(NSSet* touches, UIEvent* event)
{
	m_leftAxisX = 0.0f;
	m_leftAxisY = 0.0f;
}

void InputDeviceTouch::touchesCancelled(NSSet* touches, UIEvent* event)
{
	m_leftAxisX = 0.0f;
	m_leftAxisY = 0.0f;
}

	}
}
