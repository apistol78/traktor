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
:	m_landscape(false)
,	m_center(0.0f)
{
}

bool InputDeviceTouch::create(void* nativeWindowHandle)
{
	UIView* view = (UIView*)nativeWindowHandle;
	CGRect frame = [view frame];
	
	UITouchView* touchView = [[UITouchView alloc] initWithFrame: frame];
	[touchView setCallback: this];
	touchView.multipleTouchEnabled = YES;
	
	[view addSubview: touchView];
	
	m_landscape = true;
	m_center = frame.origin.y + frame.size.height / 2;
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
		return m_leftPad.axisX;
	else if (control == -2)
		return m_leftPad.axisY;
	else if (control == -3)
		return m_rightPad.axisX;
	else if (control == -4)
		return m_rightPad.axisY;
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
		
	case DtThumbRightX:
		control = -3;
		break;
		
	case DtThumbRightY:
		control = -4;
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
	for (UITouch* touch in touches)
	{
		CGPoint location = [touch locationInView: nil];
		if (location.y > m_center)
		{
			m_leftPad.touch = touch;
			m_leftPad.origin = location;
		}
		else
		{
			m_rightPad.touch = touch;
			m_rightPad.origin = location;
		}
	}
}

void InputDeviceTouch::touchesMoved(NSSet* touches, UIEvent* event)
{
	Pad* pad;
	for (UITouch* touch in touches)
	{
		CGPoint location = [touch locationInView: nil];
		
		if (touch == m_leftPad.touch)
			pad = &m_leftPad;
		else if (touch == m_rightPad.touch)
			pad = &m_rightPad;
		else
			continue;
		
		float offsetX = location.x - pad->origin.x;
		float offsetY = location.y - pad->origin.y;
		
		if (m_landscape)
			std::swap(offsetX, offsetY);
			
		pad->axisX = -clamp(offsetX / 30.0f, -1.0f, 1.0f);
		pad->axisY = -clamp(offsetY / 30.0f, -1.0f, 1.0f);
	}
}

void InputDeviceTouch::touchesEnded(NSSet* touches, UIEvent* event)
{
	m_leftPad.axisX =
	m_leftPad.axisY =
	m_rightPad.axisX =
	m_rightPad.axisY = 0.0f;
	m_leftPad.touch =
	m_rightPad.touch = 0;
}

void InputDeviceTouch::touchesCancelled(NSSet* touches, UIEvent* event)
{
	m_leftPad.axisX =
	m_leftPad.axisY =
	m_rightPad.axisX =
	m_rightPad.axisY = 0.0f;
	m_leftPad.touch =
	m_rightPad.touch = 0;
}

	}
}
