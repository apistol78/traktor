/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_input_InputDeviceKeyboardOsX_H
#define traktor_input_InputDeviceKeyboardOsX_H

#import <Cocoa/Cocoa.h>
#include "Core/Containers/CircularVector.h"
#include "Input/IInputDevice.h"

namespace traktor
{
	namespace input
	{
	
class InputDeviceKeyboardOsX : public IInputDevice
{
	T_RTTI_CLASS;
	
public:
	InputDeviceKeyboardOsX();

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
	
	void consumeEvent(NSEvent* event);

private:
	uint8_t m_data[256];
	CircularVector< KeyEvent, 16 > m_keyEvents;
};
	
	}
}

#endif	// traktor_input_InputDeviceKeyboardOsX_H
