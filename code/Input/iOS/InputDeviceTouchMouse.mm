#import <UIKit/UIKit.h>

#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Input/iOS/InputDeviceTouchMouse.h"

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

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputDeviceTouchMouse", InputDeviceTouchMouse, IInputDevice)

InputDeviceTouchMouse::InputDeviceTouchMouse()
:	m_landscape(false)
,	m_width(0.0f)
,	m_height(0.0f)
,	m_positionX(0.0f)
,	m_positionY(0.0f)
,	m_button(false)
{
}

bool InputDeviceTouchMouse::create(void* nativeWindowHandle)
{
	UIView* view = (UIView*)nativeWindowHandle;
	CGRect frame = [view frame];

	m_width = frame.size.width;
	m_height = frame.size.height;

	UIInterfaceOrientation orientation = [UIApplication sharedApplication].statusBarOrientation;
	m_landscape = UIInterfaceOrientationIsLandscape(orientation);

	return true;
}

std::wstring InputDeviceTouchMouse::getName() const
{
	return L"Touch Mouse";
}

InputCategory InputDeviceTouchMouse::getCategory() const
{
	return CtMouse;
}

bool InputDeviceTouchMouse::isConnected() const
{
	return true;
}

int32_t InputDeviceTouchMouse::getControlCount()
{
	return sizeof_array(c_mouseControlMap);
}

std::wstring InputDeviceTouchMouse::getControlName(int32_t control)
{
	return c_mouseControlMap[control].name;
}

bool InputDeviceTouchMouse::isControlAnalogue(int32_t control) const
{
	return c_mouseControlMap[control].analogue;
}

bool InputDeviceTouchMouse::isControlStable(int32_t control) const
{
	return false;
}

float InputDeviceTouchMouse::getControlValue(int32_t control)
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

bool InputDeviceTouchMouse::getControlRange(int32_t control, float& outMin, float& outMax) const
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

bool InputDeviceTouchMouse::getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const
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

bool InputDeviceTouchMouse::getKeyEvent(KeyEvent& outEvent)
{
	return false;
}

void InputDeviceTouchMouse::resetState()
{
	m_positionX = 0.0f;
	m_positionY = 0.0f;
	m_button = false;
}

void InputDeviceTouchMouse::readState()
{
	UIInterfaceOrientation orientation = [UIApplication sharedApplication].statusBarOrientation;
	m_landscape = UIInterfaceOrientationIsLandscape(orientation);
}

bool InputDeviceTouchMouse::supportRumble() const
{
	return false;
}

void InputDeviceTouchMouse::setRumble(const InputRumble& rumble)
{
}

void InputDeviceTouchMouse::setExclusive(bool exclusive)
{
}

void InputDeviceTouchMouse::touchesBegan(NSSet* touches, UIEvent* event)
{
	for (UITouch* touch in touches)
	{
		CGPoint location = [touch locationInView: nil];

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
}

void InputDeviceTouchMouse::touchesMoved(NSSet* touches, UIEvent* event)
{
	for (UITouch* touch in touches)
	{
		CGPoint location = [touch locationInView: nil];

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

void InputDeviceTouchMouse::touchesEnded(NSSet* touches, UIEvent* event)
{
	for (UITouch* touch in touches)
	{
		CGPoint location = [touch locationInView: nil];

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
	}
}

void InputDeviceTouchMouse::touchesCancelled(NSSet* touches, UIEvent* event)
{
	m_button = false;
}

	}
}
