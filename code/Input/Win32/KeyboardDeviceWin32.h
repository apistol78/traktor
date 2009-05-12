#ifndef traktor_input_KeyboardDeviceWin32_H
#define traktor_input_KeyboardDeviceWin32_H

#include "Input/InputDevice.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_WIN32_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class T_DLLCLASS KeyboardDeviceWin32 : public InputDevice
{
	T_RTTI_CLASS(KeyboardDeviceWin32)

public:
	virtual std::wstring getName() const;

	virtual InputCategory getCategory() const;

	virtual int getControlCount();

	virtual std::wstring getControlName(int control);

	virtual bool isControlAnalogue(int control) const;

	virtual float getControlValue(int control);

	virtual bool getDefaultControl(InputDefaultControlType controlType, int& control) const;

	virtual void resetState();

	virtual void readState();

	virtual bool supportRumble() const;

	virtual void setRumble(const InputRumble& rumble);
};

	}
}

#endif	// traktor_input_KeyboardDeviceDi8_H
