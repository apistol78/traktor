#ifndef traktor_input_InputDriverDi8_H
#define traktor_input_InputDriverDi8_H

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include "Core/RefArray.h"
#include "Core/Misc/ComRef.h"
#include "Input/IInputDriver.h"

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

class InputDeviceDi8;

class T_DLLCLASS InputDriverDi8 : public IInputDriver
{
	T_RTTI_CLASS;

public:
	InputDriverDi8();

	virtual ~InputDriverDi8();

	bool create(HWND hWnd);

	void destroy();

	virtual int getDeviceCount();

	virtual Ref< IInputDevice > getDevice(int index);

private:
	static BOOL CALLBACK enumDevicesCallback(const DIDEVICEINSTANCE* deviceInstance, VOID* context);

	bool addDevice(const DIDEVICEINSTANCE* instance);

	HWND m_hWnd;
	RefArray< InputDeviceDi8 > m_devices;
	ComRef< IDirectInput8 > m_directInput;
};

	}
}

#endif	// traktor_input_InputDriverDi8_H
