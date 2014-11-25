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
	int m_padIndex;
	CellPadData m_padData;
	CellPadActParam m_padActuator;
	bool m_connected;
};

	}
}

#endif	// traktor_input_InputDevicePs3_H
