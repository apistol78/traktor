#ifndef traktor_input_KeyboardDeviceDi8_H
#define traktor_input_KeyboardDeviceDi8_H

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include "Input/IInputDevice.h"
#include "Core/Misc/ComRef.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_DI8_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class T_DLLCLASS KeyboardDeviceDi8 : public IInputDevice
{
	T_RTTI_CLASS;

public:
	KeyboardDeviceDi8(IDirectInputDevice8* diDevice, const DIDEVICEINSTANCE* deviceInstance);

	virtual std::wstring getName() const;

	virtual InputCategory getCategory() const;

	virtual bool isConnected() const;

	virtual int32_t getControlCount();

	virtual std::wstring getControlName(int32_t control);

	virtual bool isControlAnalogue(int32_t control) const;

	virtual int32_t getActiveControlCount() const;

	virtual float getControlValue(int32_t control);

	virtual bool getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const;

	virtual void resetState();

	virtual void readState();

	virtual bool supportRumble() const;

	virtual void setRumble(const InputRumble& rumble);

private:
	ComRef< IDirectInputDevice8 > m_device;
	std::wstring m_name;
	uint8_t m_state[256];
	bool m_connected;
};

	}
}

#endif	// traktor_input_KeyboardDeviceDi8_H
