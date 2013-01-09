#ifndef traktor_input_KeyboardDeviceX11_H
#define traktor_input_KeyboardDeviceX11_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XInput2.h>
#include "Core/Containers/CircularVector.h"
#include "Input/IInputDevice.h"

namespace traktor
{
	namespace input
	{

class KeyboardDeviceX11 : public IInputDevice
{
	T_RTTI_CLASS;

public:
	KeyboardDeviceX11();

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
	bool m_connected;
	CircularVector< KeyEvent, 16 > m_keyEvents;
	uint8_t m_keyStates[256];
};

	}
}

#endif	// traktor_input_KeyboardDeviceX11_H
