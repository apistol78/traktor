#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Input/OsX/InputDeviceMouseOsX.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

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
	return 0;
}

std::wstring InputDeviceMouseOsX::getControlName(int32_t control)
{
	switch (control)
	{
	case -1:
		return L"Axis X";
	case -2:
		return L"Axis Y";
	case -3:
		return L"Wheel";
	}
	return L"Button " + toString(1 + control);
}

bool InputDeviceMouseOsX::isControlAnalogue(int32_t control) const
{
	return control < 0;
}

float InputDeviceMouseOsX::getControlValue(int32_t control)
{
	if (control == -1)
		return m_axis[0];
	else if (control == -2)
		return m_axis[1];
	else if (control == -3)
		return m_axis[2];
	else if (control >= 0 && control < sizeof_array(m_button))
		return m_button[control] ? 1.0f : 0.0f;
	else
		return 0.0f;
}

bool InputDeviceMouseOsX::getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const
{
	if (analogue)
	{
		switch (controlType)
		{
		case DtAxisX:
			control = -1;
			break;
		
		case DtAxisY:
			control = -2;
			break;
			
		case DtAxisZ:
			control = -3;
			break;

		default:
			return false;
		}
	}
	else
	{
		switch (controlType)
		{
		case DtButton1:
			control = 0;
			break;
		
		case DtButton2:
			control = 1;
			break;
		
		case DtButton3:
			control = 2;
			break;
		
		case DtButton4:
			control = 3;
			break;
		
		default:
			return false;
		}
	}

	return true;
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

	CFArrayRef elements = IOHIDDeviceCopyMatchingElements(m_deviceRef, NULL, kIOHIDOptionsTypeNone);

	CFIndex elementCount = CFArrayGetCount(elements);
	for (CFIndex i = 0; i < elementCount; ++i)
	{
		IOHIDElementRef e = (IOHIDElementRef)CFArrayGetValueAtIndex(elements, i);
		if (!e)
			continue;

		uint32_t usage = (uint32_t)IOHIDElementGetUsage(e);
		if (usage == ~0UL)
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
}

bool InputDeviceMouseOsX::supportRumble() const
{
	return false;
}

void InputDeviceMouseOsX::setRumble(const InputRumble& rumble)
{
}

void InputDeviceMouseOsX::callbackRemoval(void* context, IOReturn result, void* sender)
{
	InputDeviceMouseOsX* this_ = static_cast< InputDeviceMouseOsX* >(context);
	this_->m_deviceRef = 0;
}

	}
}
