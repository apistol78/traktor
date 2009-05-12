#ifndef traktor_input_InputDevicePs3_H
#define traktor_input_InputDevicePs3_H

#include <cell/pad.h>
#include "Input/InputDevice.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_PS3_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class T_DLLCLASS InputDevicePs3 : public InputDevice
{
	T_RTTI_CLASS(InputDevicePs3)

public:
	InputDevicePs3(int padIndex);

	virtual std::string getName() const;

	virtual Category getCategory() const;

	virtual int getControlCount();

	virtual std::string getControlName(int control);

	virtual bool isControlAnalogue(int control) const;

	virtual float getControlValue(int control);

	virtual bool getDefaultControl(DefaultControlType controlType, int& control) const;

	virtual void readState();

private:
	bool m_enabled;
	int m_padIndex;
	CellPadData m_padData;
};

	}
}

#endif	// traktor_input_InputDevicePs3_H
