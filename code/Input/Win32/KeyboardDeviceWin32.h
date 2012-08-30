#ifndef traktor_input_KeyboardDeviceWin32_H
#define traktor_input_KeyboardDeviceWin32_H

#include "Input/IInputDevice.h"
#include "Input/Win32/TypesWin32.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_WIN32_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class T_DLLCLASS KeyboardDeviceWin32 : public IInputDevice
{
	T_RTTI_CLASS;

public:
	KeyboardDeviceWin32();

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

	virtual void resetState();

	virtual void readState();

	virtual bool supportRumble() const;

	virtual void setRumble(const InputRumble& rumble);

	virtual void setExclusive(bool exclusive);

private:
	friend class InputDriverWin32;

	bool m_connected;
	uint8_t m_keyStates[sizeof_array(c_vkControlKeys)];
};

	}
}

#endif	// traktor_input_KeyboardDeviceWin32_H
