#ifndef traktor_input_InputDeviceDi8_H
#define traktor_input_InputDeviceDi8_H

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include "Input/IInputDevice.h"
#include "Core/Misc/ComRef.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_DI8_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class T_DLLCLASS InputDeviceDi8 : public IInputDevice
{
	T_RTTI_CLASS;

public:
	InputDeviceDi8(IDirectInputDevice8* diDevice);

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
	ComRef< IDirectInputDevice8 > m_device;
	DIDEVICEINSTANCE m_deviceDesc;
	LPVOID m_state;
	bool m_connected;
};

	}
}

#endif	// traktor_input_InputDeviceDi8_H
