#ifndef traktor_input_InputDeviceXi_H
#define traktor_input_InputDeviceXi_H

#if defined(_XBOX)
#include <xtl.h>
#else
#include <windows.h>
#include <xinput.h>
#endif
#include "Input/IInputDevice.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_XI_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class T_DLLCLASS InputDeviceXi : public IInputDevice
{
	T_RTTI_CLASS;

public:
	InputDeviceXi(DWORD controller);

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
	DWORD m_controller;
	XINPUT_STATE m_state;
	bool m_connected;
	uint32_t m_skip;
};

	}
}

#endif	// traktor_input_InputDeviceXi_H
