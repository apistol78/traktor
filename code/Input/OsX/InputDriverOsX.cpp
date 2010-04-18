#include "Core/Log/Log.h"
#include "Input/OsX/InputDeviceOsX.h"
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

bool InputDriverOsX::create()
{
	IOHIDManagerRef managerRef = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
	if (!managerRef)
		return false;
	
	CFDictionaryRef matchingCFDictRef = createMatchingDictionary(kHIDPage_GenericDesktop, kHIDUsage_GD_GamePad);
	if (!matchingCFDictRef)
		return false;

	IOHIDManagerSetDeviceMatching(managerRef, matchingCFDictRef);
	IOHIDManagerRegisterDeviceMatchingCallback(managerRef, callbackDeviceMatch, this);
//	IOHIDManagerRegisterDeviceRemovalCallback(tIOHIDManagerRef, Handle_RemovalCallback, &sSharedStickHandler);
	IOHIDManagerScheduleWithRunLoop(managerRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	
	return true;
}

void InputDriverOsX::destroy()
{
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
	if (!m_devicesChanged)
		return UrOk;
	
	m_devicesChanged = false;
	return UrDevicesChanged;
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
	
	Ref< InputDeviceOsX > device = new InputDeviceOsX(inIOHIDDeviceRef);
	this_->m_devices.push_back(device);
	this_->m_devicesChanged = true;
}

	}
}
