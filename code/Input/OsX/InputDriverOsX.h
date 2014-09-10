#ifndef traktor_input_InputDriverOsX_H
#define traktor_input_InputDriverOsX_H

#include <IOKit/hid/IOHIDLib.h>
#include "Core/RefArray.h"
#include "Input/IInputDriver.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_OSX_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{
	
class IInputDevice;

class T_DLLCLASS InputDriverOsX : public IInputDriver
{
	T_RTTI_CLASS;

public:
	InputDriverOsX();

	virtual bool create(void* nativeHandle, const SystemWindow& systemWindow, uint32_t inputCategories);
	
	virtual int getDeviceCount();

	virtual Ref< IInputDevice > getDevice(int index);
	
	virtual UpdateResult update();
	
private:
	bool m_devicesChanged;
	RefArray< IInputDevice > m_devices;
	
	static void callbackDeviceMatch(void* inContext, IOReturn inResult, void* inSender, IOHIDDeviceRef inIOHIDDeviceRef);
};

	}
}

#endif	// traktor_input_InputDriverOsX_H
