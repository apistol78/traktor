#ifndef traktor_input_InputDeviceKeyboardOsX_H
#define traktor_input_InputDeviceKeyboardOsX_H

#include <IOKit/hid/IOHIDLib.h>
#include "Core/Misc/AutoPtr.h"
#include "Input/IInputDevice.h"

namespace traktor
{
	namespace input
	{
	
class InputDeviceKeyboardOsX : public IInputDevice
{
	T_RTTI_CLASS;
	
public:
	InputDeviceKeyboardOsX(IOHIDDeviceRef deviceRef);

	virtual std::wstring getName() const;

	virtual InputCategory getCategory() const;

	virtual bool isConnected() const;

	virtual int32_t getControlCount();

	virtual std::wstring getControlName(int32_t control);

	virtual bool isControlAnalogue(int32_t control) const;

	virtual float getControlValue(int32_t control);

	virtual bool getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const;

	virtual void resetState();

	virtual void readState();

	virtual bool supportRumble() const;

	virtual void setRumble(const InputRumble& rumble);
	
private:
	IOHIDDeviceRef m_deviceRef;
	AutoArrayPtr< uint8_t > m_data;
	
	static void callbackRemoval(void* context, IOReturn result, void* sender);
};
	
	}
}

#endif	// traktor_input_InputDeviceKeyboardOsX_H
