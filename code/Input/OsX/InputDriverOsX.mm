#import <Cocoa/Cocoa.h>

#include "Core/Platform.h"
#include "Core/Log/Log.h"
#include "Input/OsX/InputDeviceGamepadOsX.h"
#include "Input/OsX/InputDeviceJoystickOsX.h"
#include "Input/OsX/InputDeviceKeyboardOsX.h"
#include "Input/OsX/InputDeviceMouseOsX.h"
#include "Input/OsX/InputDriverOsX.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

CFMutableDictionaryRef createMatchingDictionary(UInt32 inUsagePage, UInt32 inUsage)
{
	CFMutableDictionaryRef result = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	if (!result)
		return 0;
	
	if (inUsagePage && inUsage)
	{
		CFNumberRef pageCFNumberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &inUsagePage);
		if (!pageCFNumberRef)
			return 0;
			
		CFDictionarySetValue(result, CFSTR(kIOHIDDeviceUsagePageKey), pageCFNumberRef);
		CFRelease(pageCFNumberRef);
		
		CFNumberRef usageCFNumberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &inUsage);
		if (!usageCFNumberRef)
			return 0;
			
		CFDictionarySetValue(result, CFSTR(kIOHIDDeviceUsageKey), usageCFNumberRef);
		CFRelease(usageCFNumberRef);
	}
	
	return result;
}

		}
	
T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InputDriverOsX", 0, InputDriverOsX, IInputDriver)

InputDriverOsX::InputDriverOsX()
:	m_devicesChanged(false)
{
}

bool InputDriverOsX::create(const SystemApplication& sysapp, const SystemWindow& syswin, uint32_t inputCategories)
{
	// Create vanilla devices.
	m_keyboardDevice = new InputDeviceKeyboardOsX();
	m_mouseDevice = new InputDeviceMouseOsX();

	m_devices.push_back(m_keyboardDevice);
	m_devices.push_back(m_mouseDevice);
	m_devicesChanged = true;

	// Install a local monitor of all keyboard activitiy.
	[NSEvent addLocalMonitorForEventsMatchingMask: NSKeyUpMask | NSKeyDownMask | NSScrollWheelMask handler:^ NSEvent* (NSEvent* event)
	{
		m_keyboardDevice->consumeEvent(event);
		m_mouseDevice->consumeEvent(event);
		return event;
	}];

	// Use HID to access joystick and gamepads.
	if (inputCategories & CtJoystick)
	{
		IOHIDManagerRef managerRef = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
		if (!managerRef)
		{
			log::error << L"Unable to create input driver; IOHIDManagerCreate failed" << Endl;
			return false;
		}
	
		CFArrayRef matchingDictionariesRef = CFArrayCreateMutable(
			kCFAllocatorDefault,
			0,
			&kCFTypeArrayCallBacks
		);

		// Gamepad matching dictionary.
		if (inputCategories & CtJoystick)
		{
			CFDictionaryRef matchingCFDictRef = createMatchingDictionary(
				kHIDPage_GenericDesktop,
				kHIDUsage_GD_GamePad
			);
			if (!matchingCFDictRef)
			{
				log::error << L"Unable to create input driver; failed to create gamepad matching dictionary" << Endl;
				return false;
			}
			CFArrayAppendValue((__CFArray*)matchingDictionariesRef, matchingCFDictRef);
			CFRelease(matchingCFDictRef);
		}

		// Joystick matching dictionary.
		if (inputCategories & CtJoystick)
		{
			CFDictionaryRef matchingCFDictRef = createMatchingDictionary(
				kHIDPage_GenericDesktop,
				kHIDUsage_GD_Joystick
			);
			if (!matchingCFDictRef)
			{
				log::error << L"Unable to create input driver; failed to create joystick matching dictionary" << Endl;
				return false;
			}
			CFArrayAppendValue((__CFArray*)matchingDictionariesRef, matchingCFDictRef);
			CFRelease(matchingCFDictRef);
		}
		
		IOHIDManagerSetDeviceMatchingMultiple(managerRef, matchingDictionariesRef);
		IOHIDManagerRegisterDeviceMatchingCallback(managerRef, callbackDeviceMatch, this);
		IOHIDManagerScheduleWithRunLoop(managerRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	}

	return true;
}

int InputDriverOsX::getDeviceCount()
{
	return int(m_devices.size());
}

Ref< IInputDevice > InputDriverOsX::getDevice(int index)
{
	return m_devices[index];
}

IInputDriver::UpdateResult InputDriverOsX::update()
{
	if (m_devicesChanged)
	{
		m_devicesChanged = false;
		return UrDevicesChanged;
	}
	return UrOk;
}

void InputDriverOsX::callbackDeviceMatch(void* inContext, IOReturn inResult, void* inSender, IOHIDDeviceRef inIOHIDDeviceRef)
{
	InputDriverOsX* this_ = (InputDriverOsX*)inContext;
	T_ASSERT (this_);

	IOHIDManagerRef managerRef = (IOHIDManagerRef)inSender;
	T_ASSERT (managerRef);
	
	IOReturn result = IOHIDManagerOpen(managerRef, kIOHIDOptionsTypeNone);
	if (result != kIOReturnSuccess)
		return;
		
	if (IOHIDDeviceConformsTo(inIOHIDDeviceRef, kHIDPage_GenericDesktop, kHIDUsage_GD_GamePad))
	{
		log::info << L"HID device; matching gamepad device connected" << Endl;

		Ref< InputDeviceGamepadOsX > device = new InputDeviceGamepadOsX(inIOHIDDeviceRef);
		this_->m_devices.push_back(device);
		this_->m_devicesChanged = true;
	}
	else if (IOHIDDeviceConformsTo(inIOHIDDeviceRef, kHIDPage_GenericDesktop, kHIDUsage_GD_Joystick))
	{
		log::info << L"HID device; matching joystick device connected" << Endl;

		Ref< InputDeviceJoystickOsX > device = new InputDeviceJoystickOsX(inIOHIDDeviceRef);
		this_->m_devices.push_back(device);
		this_->m_devicesChanged = true;
	}
}

	}
}
