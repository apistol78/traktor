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

	virtual int getControlCount();

	virtual std::wstring getControlName(int control);

	virtual bool isControlAnalogue(int control) const;

	virtual float getControlValue(int control);

	virtual bool getDefaultControl(InputDefaultControlType controlType, int& control) const;

	virtual void resetState();

	virtual void readState();

	virtual bool supportRumble() const;

	virtual void setRumble(const InputRumble& rumble);
	
private:
	IOHIDDeviceRef m_deviceRef;
	AutoArrayPtr< uint8_t > m_data;
};
	
	}
}

#endif	// traktor_input_InputDeviceKeyboardOsX_H
