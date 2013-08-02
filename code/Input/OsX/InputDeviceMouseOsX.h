#ifndef traktor_input_InputDeviceMouseOsX_H
#define traktor_input_InputDeviceMouseOsX_H

#include <IOKit/hid/IOHIDLib.h>
#include "Input/IInputDevice.h"

namespace traktor
{
	namespace input
	{
	
class InputDeviceMouseOsX : public IInputDevice
{
	T_RTTI_CLASS;
	
public:
	InputDeviceMouseOsX(IOHIDDeviceRef deviceRef);

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
	bool m_button[4];
	int32_t m_axis[5];
	uint64_t m_timeStamps[3];
	bool m_exclusive;
	bool m_lastMouseValid;
	
	static void callbackRemoval(void* context, IOReturn result, void* sender);
};
	
	}
}

#endif	// traktor_input_InputDeviceMouseOsX_H
