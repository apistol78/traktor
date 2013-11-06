#import <ApplicationServices/ApplicationServices.h>
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Input/OsX/InputDeviceMouseOsX.h"
#include "Input/OsX/InputUtilities.h"

namespace traktor
{
	namespace input
	{
		namespace
		{
		
struct MouseControlMap
{
	InputDefaultControlType control;
	int32_t index;
	bool analogue;
	bool stable;
	const wchar_t* name;
}
c_mouseControlMap[] =
{
	{ DtAxisX, -1, true, false, L"Axis X" },
	{ DtAxisY, -2, true, false, L"Axis Y" },
	{ DtAxisZ, -3, true, false, L"Wheel" },
	{ DtButton1, 0, false, true, L"Button 1" },
	{ DtButton2, 1, false, true, L"Button 2" },
	{ DtButton3, 2, false, true, L"Button 3" },
	{ DtButton4, 3, false, true, L"Button 4" },
	{ DtPositionX, -4, true, false, L"Axis X" },
	{ DtPositionY, -5, true, false, L"Axis Y" }
};

int32_t getElementValue(IOHIDDeviceRef deviceRef, IOHIDElementRef elementRef)
{
	IOHIDValueRef valueRef = 0;
	IOHIDDeviceGetValue(deviceRef, elementRef, &valueRef);
	if (!valueRef)
		return 0;
		
	int32_t value = (int32_t)IOHIDValueGetIntegerValue(valueRef);
	return value;
}

int32_t getElementValue(IOHIDDeviceRef deviceRef, IOHIDElementRef elementRef, uint64_t& lastTimeStamp)
{
	IOHIDValueRef valueRef = 0;
	IOHIDDeviceGetValue(deviceRef, elementRef, &valueRef);
	if (!valueRef)
		return 0;
		
	uint64_t timeStamp = IOHIDValueGetTimeStamp(valueRef);
	if (timeStamp == lastTimeStamp)
		return 0;
		
	int32_t value = (int32_t)IOHIDValueGetIntegerValue(valueRef);
	lastTimeStamp = timeStamp;
	
	return value;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputDeviceMouseOsX", InputDeviceMouseOsX, IInputDevice)

InputDeviceMouseOsX::InputDeviceMouseOsX(IOHIDDeviceRef deviceRef)
:	m_deviceRef(deviceRef)
,	m_exclusive(false)
,	m_lastMouseValid(false)
{
	if (m_deviceRef)
		IOHIDDeviceRegisterRemovalCallback(m_deviceRef, &callbackRemoval, this);
		
	std::memset(m_timeStamps, 0, sizeof(m_timeStamps));

	resetState();
}

std::wstring InputDeviceMouseOsX::getName() const
{
	return L"HID Mouse";
}

InputCategory InputDeviceMouseOsX::getCategory() const
{
	return CtMouse;
}

bool InputDeviceMouseOsX::isConnected() const
{
	return m_deviceRef != 0;
}

int32_t InputDeviceMouseOsX::getControlCount()
{
	return sizeof_array(c_mouseControlMap);
}

std::wstring InputDeviceMouseOsX::getControlName(int32_t control)
{
	return c_mouseControlMap[control].name;
}

bool InputDeviceMouseOsX::isControlAnalogue(int32_t control) const
{
	return c_mouseControlMap[control].analogue;
}

bool InputDeviceMouseOsX::isControlStable(int32_t control) const
{
	return c_mouseControlMap[control].stable;
}

float InputDeviceMouseOsX::getControlValue(int32_t control)
{
	int32_t index = c_mouseControlMap[control].index;
	if (index == -1)
		return m_axis[0];
	else if (index == -2)
		return m_axis[1];
	else if (index == -3)
		return m_axis[2];
    else if (index == -4)
        return m_axis[3];
    else if (index == -5)
        return m_axis[4];
	else if (index >= 0 && index < sizeof_array(m_button))
		return m_button[index] ? 1.0f : 0.0f;
	else
		return 0.0f;
}

bool InputDeviceMouseOsX::getControlRange(int32_t control, float& outMin, float& outMax) const
{
	int32_t index = c_mouseControlMap[control].index;
	float dummy;
	
	if (index == -4)
	{
		outMin = 0.0f;
		return getMouseRange(outMax, dummy);
	}
	else if (index == -5)
	{
		outMin = 0.0f;
		return getMouseRange(dummy, outMax);
	}
	else
		return false;
		
	return true;
}

bool InputDeviceMouseOsX::getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const
{
	for (int32_t i = 0; i < sizeof_array(c_mouseControlMap); ++i)
	{
		const MouseControlMap& mc = c_mouseControlMap[i];
		if (mc.control == controlType && mc.analogue == analogue)
		{
			control = i;
			return true;
		}
	}
	return false;
}

bool InputDeviceMouseOsX::getKeyEvent(KeyEvent& outEvent)
{
	return false;
}

void InputDeviceMouseOsX::resetState()
{
	std::memset(m_button, 0, sizeof(m_button));
	std::memset(m_axis, 0, sizeof(m_axis));
}

void InputDeviceMouseOsX::readState()
{
	resetState();
	
	if (!m_deviceRef)
		return;

	bool mouseValid = isInputAllowed();
		
	CFArrayRef elements = IOHIDDeviceCopyMatchingElements(m_deviceRef, NULL, kIOHIDOptionsTypeNone);

	CFIndex elementCount = CFArrayGetCount(elements);
	for (CFIndex i = 0; i < elementCount; ++i)
	{
		IOHIDElementRef e = (IOHIDElementRef)CFArrayGetValueAtIndex(elements, i);
		if (!e)
			continue;

		uint32_t usage = (uint32_t)IOHIDElementGetUsage(e);
		if (usage == uint32_t(~0UL))
			continue;
			
		uint32_t page = (uint32_t)IOHIDElementGetUsagePage(e);
		
		if (page == kHIDPage_GenericDesktop)
		{
			if (usage == kHIDUsage_GD_X)
				m_axis[0] = getElementValue(m_deviceRef, e, m_timeStamps[0]) * 2.0f;
			else if (usage == kHIDUsage_GD_Y)
				m_axis[1] = getElementValue(m_deviceRef, e, m_timeStamps[1]) * 2.0f;
			else if (usage == kHIDUsage_GD_Wheel)
				m_axis[2] = getElementValue(m_deviceRef, e, m_timeStamps[2]);
		}
		else if (page == kHIDPage_Button)
		{
			if (usage == kHIDUsage_Button_1)
			{
				int32_t value = getElementValue(m_deviceRef, e);
				m_button[0] |= bool(value != 0);
			}
			else if (usage == kHIDUsage_Button_2)
			{
				int32_t value = getElementValue(m_deviceRef, e);
				m_button[1] |= bool(value != 0);
			}
			else if (usage == kHIDUsage_Button_3)
			{
				int32_t value = getElementValue(m_deviceRef, e);
				m_button[2] |= bool(value != 0);
			}
			else if (usage == kHIDUsage_Button_4)
			{
				int32_t value = getElementValue(m_deviceRef, e);
				m_button[3] |= bool(value != 0);
			}
		}
	}
	
	CFRelease(elements);
    
    // Get position of mouse relative to key window.
    float mouseX, mouseY;
    if (getMousePosition(mouseX, mouseY))
    {
        m_axis[3] = int32_t(mouseX);
        m_axis[4] = int32_t(mouseY);
    }
    else
    {
        m_axis[3] =
        m_axis[4] = 0;
    }
	
	// If mouse capture is exclusive then move cursor to center of key window.
	if (m_exclusive)
	{
		float centerX, centerY;
		if (getMouseCenterPosition(centerX, centerY))
			CGWarpMouseCursorPosition(CGPointMake(centerX, centerY));
	}
	
	// As long as user keps mouse button pressed we cannot
	// leave invalid state.
	if (mouseValid && !m_lastMouseValid)
	{
		if (m_button[0] || m_button[1] || m_button[2])
			mouseValid = false;
	}
	
	// Discard all inputs if mouse not in a valid
	// region.
	if (!mouseValid)
		resetState();
	
	m_lastMouseValid = mouseValid;
}

bool InputDeviceMouseOsX::supportRumble() const
{
	return false;
}

void InputDeviceMouseOsX::setRumble(const InputRumble& rumble)
{
}

void InputDeviceMouseOsX::setExclusive(bool exclusive)
{
	m_exclusive = exclusive;
}

void InputDeviceMouseOsX::callbackRemoval(void* context, IOReturn result, void* sender)
{
	InputDeviceMouseOsX* this_ = static_cast< InputDeviceMouseOsX* >(context);
	this_->m_deviceRef = 0;
}

	}
}
