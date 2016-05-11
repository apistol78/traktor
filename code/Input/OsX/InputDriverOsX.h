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
class InputDeviceKeyboardOsX;
class InputDeviceMouseOsX;

class T_DLLCLASS InputDriverOsX : public IInputDriver
{
	T_RTTI_CLASS;

public:
	InputDriverOsX();

	virtual bool create(const SystemApplication& sysapp, const SystemWindow& syswin, uint32_t inputCategories);
	
	virtual int getDeviceCount();

	virtual Ref< IInputDevice > getDevice(int index);
	
	virtual UpdateResult update();

private:
	bool m_devicesChanged;
	RefArray< IInputDevice > m_devices;
	Ref< InputDeviceKeyboardOsX > m_keyboardDevice;
	Ref< InputDeviceMouseOsX > m_mouseDevice;

	static void callbackDeviceMatch(void* inContext, IOReturn inResult, void* inSender, IOHIDDeviceRef inIOHIDDeviceRef);
};

	}
}

#endif	// traktor_input_InputDriverOsX_H
