#ifndef traktor_input_InputDriverWin32_H
#define traktor_input_InputDriverWin32_H

#include "Input/IInputDriver.h"

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

class KeyboardDeviceWin32;
class MouseDeviceWin32;

class T_DLLCLASS InputDriverWin32 : public IInputDriver
{
	T_RTTI_CLASS;

public:
	InputDriverWin32();

	virtual int getDeviceCount();

	virtual Ref< IInputDevice > getDevice(int index);

private:
	Ref< KeyboardDeviceWin32 > m_keyboardDevice;
	Ref< MouseDeviceWin32 > m_mouseDevice;
};

	}
}

#endif	// traktor_input_InputDriverWin32_H
