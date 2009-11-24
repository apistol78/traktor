#ifndef traktor_input_InputDevicePs3_H
#define traktor_input_InputDevicePs3_H

#include <cell/pad.h>
#include "Input/IInputDevice.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_PS3_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class T_DLLCLASS InputDevicePs3 : public IInputDevice
{
	T_RTTI_CLASS;

public:
	InputDevicePs3(int padIndex);

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
	bool m_enabled;
	int m_padIndex;
	CellPadData m_padData;
};

	}
}

#endif	// traktor_input_InputDevicePs3_H
