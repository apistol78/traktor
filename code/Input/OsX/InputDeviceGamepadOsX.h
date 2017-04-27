/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_input_InputDeviceGamepadOsX_H
#define traktor_input_InputDeviceGamepadOsX_H

#include <IOKit/hid/IOHIDLib.h>
#include "Input/IInputDevice.h"

namespace traktor
{
	namespace input
	{
	
class InputDeviceGamepadOsX : public IInputDevice
{
	T_RTTI_CLASS;
	
public:
	InputDeviceGamepadOsX(IOHIDDeviceRef deviceRef);

	virtual std::wstring getName() const;

	virtual InputCategory getCategory() const;

	virtual bool isConnected() const;

	virtual int32_t getControlCount();

	virtual std::wstring getControlName(int32_t control);

	virtual bool isControlAnalogue(int32_t control) const;
	
	virtual bool isControlStable(int32_t control) const;
	
	virtual float getControlValue(int32_t control);

	virtual bool getControlRange(int32_t control, float& outMin, float& outMax) const;

	virtual bool getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const;

	virtual bool getKeyEvent(KeyEvent& outEvent);

	virtual void resetState();

	virtual void readState();

	virtual bool supportRumble() const;

	virtual void setRumble(const InputRumble& rumble);
	
	virtual void setExclusive(bool exclusive);
	
private:
	IOHIDDeviceRef m_deviceRef;
	uint8_t m_button[14];
	float m_axis[3][2];
	
	static void callbackRemoval(void* context, IOReturn result, void* sender);

	static void callbackValue(void* context, IOReturn result, void* sender, IOHIDValueRef value);
};
	
	}
}

#endif	// traktor_input_InputDeviceGamepadOsX_H
