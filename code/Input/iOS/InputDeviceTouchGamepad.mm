#import <UIKit/UIKit.h>

#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Input/iOS/InputDeviceTouchGamepad.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputDeviceTouchGamepad", InputDeviceTouchGamepad, IInputDevice)

InputDeviceTouchGamepad::InputDeviceTouchGamepad()
:	m_landscape(false)
{
}

bool InputDeviceTouchGamepad::create(void* nativeWindowHandle)
{
	UIView* view = (UIView*)nativeWindowHandle;
	CGRect frame = [view frame];

	float cx = frame.origin.x + frame.size.width / 2.0f;
	float cy = frame.origin.y + frame.size.height / 2.0f;
	
	m_pivots[0] = CGPointMake(cx, cy - 30.0f);
	m_pivots[1] = CGPointMake(cx, cy);
	m_pivots[2] = CGPointMake(cx, cy + 30.0f);
	
	UIDeviceOrientation orientation = [UIApplication sharedApplication].statusBarOrientation;
	m_landscape = UIInterfaceOrientationIsLandscape(orientation);

	if (!m_landscape)
	{
		m_controls[0] = &m_leftButton;
		m_controls[1] = &m_rightButton;
		m_controls[2] = &m_rightPad;
		m_controls[3] = &m_leftPad;
	}
	else
	{
		m_controls[0] = &m_rightButton;
		m_controls[1] = &m_rightPad;
		m_controls[2] = &m_leftPad;
		m_controls[3] = &m_leftButton;
	}
	
	return true;
}

std::wstring InputDeviceTouchGamepad::getName() const
{
	return L"Touch";
}

InputCategory InputDeviceTouchGamepad::getCategory() const
{
	return CtJoystick;
}

bool InputDeviceTouchGamepad::isConnected() const
{
	return true;
}

int32_t InputDeviceTouchGamepad::getControlCount()
{
	return 0;
}

std::wstring InputDeviceTouchGamepad::getControlName(int32_t control)
{
	return L"";
}

bool InputDeviceTouchGamepad::isControlAnalogue(int32_t control) const
{
	return true;
}

bool InputDeviceTouchGamepad::isControlStable(int32_t control) const
{
	return true;
}

float InputDeviceTouchGamepad::getControlValue(int32_t control)
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

bool InputDeviceTouchGamepad::getControlRange(int32_t control, float& outMin, float& outMax) const
{
	return false;
}

bool InputDeviceTouchGamepad::getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const
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

bool InputDeviceTouchGamepad::getKeyEvent(KeyEvent& outEvent)
{
	return false;
}

void InputDeviceTouchGamepad::resetState()
{
}

void InputDeviceTouchGamepad::readState()
{
}

bool InputDeviceTouchGamepad::supportRumble() const
{
	return false;
}

void InputDeviceTouchGamepad::setRumble(const InputRumble& rumble)
{
}

void InputDeviceTouchGamepad::setExclusive(bool exclusive)
{
}

void InputDeviceTouchGamepad::touchesBegan(NSSet* touches, UIEvent* event)
{
	for (UITouch* touch in touches)
	{
		CGPoint location = [touch locationInView: nil];
		IControl* control = 0;

		if (location.y < m_pivots[0].y)
		{
			if (location.x < m_pivots[0].x)
				control = m_controls[0];
			else
				control = m_controls[1];
		}
		else
		{
			if (location.x < m_pivots[0].x)
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

void InputDeviceTouchGamepad::touchesMoved(NSSet* touches, UIEvent* event)
{
	for (UITouch* touch in touches)
	{
		IControl* control = m_track[touch];
		if (control)
			control->move(this, touch);
	}
}

void InputDeviceTouchGamepad::touchesEnded(NSSet* touches, UIEvent* event)
{
	for (UITouch* touch in touches)
	{
		IControl* control = m_track[touch];
		if (control)
			control->end(touch);
	}
}

void InputDeviceTouchGamepad::touchesCancelled(NSSet* touches, UIEvent* event)
{
	for (UITouch* touch in touches)
	{
		IControl* control = m_track[touch];
		if (control)
			control->end(touch);
	}
}

void InputDeviceTouchGamepad::Pad::begin(UITouch* touch)
{
	origin = [touch locationInView: nil];
}
		
void InputDeviceTouchGamepad::Pad::end(UITouch* touch)
{
	axisX =
	axisY = 0.0f;
}
		
void InputDeviceTouchGamepad::Pad::move(InputDeviceTouchGamepad* device, UITouch* touch)
{
	CGPoint location = [touch locationInView: nil];
	
	float offsetX = location.x - origin.x;
	float offsetY = location.y - origin.y;
	
	if (device->m_landscape)
		std::swap(offsetX, offsetY);
		
	const float c_deadZone = 8.0f;
	
	if (abs(offsetX) > c_deadZone)
		offsetX = sign(offsetX) * (abs(offsetX) - c_deadZone);
	else
		offsetX = 0.0f;

	if (abs(offsetY) > c_deadZone)
		offsetY = sign(offsetY) * (abs(offsetY) - c_deadZone);
	else
		offsetY = 0.0f;
		
	axisX = -clamp(offsetX / 30.0f, -1.0f, 1.0f);
	axisY = -clamp(offsetY / 30.0f, -1.0f, 1.0f);
}

void InputDeviceTouchGamepad::Button::begin(UITouch* touch)
{
	value = 1.0f;
}
		
void InputDeviceTouchGamepad::Button::end(UITouch* touch)
{
	value = 0.0f;
}
		
void InputDeviceTouchGamepad::Button::move(InputDeviceTouchGamepad* device, UITouch* touch)
{
}

	}
}
