#ifndef traktor_input_InputDriverX11_H
#define traktor_input_InputDriverX11_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XInput2.h>
#include "Core/Platform.h"
#include "Core/RefArray.h"
#include "Input/IInputDriver.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_X11_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class InputDeviceX11;

class T_DLLCLASS InputDriverX11 : public IInputDriver
{
	T_RTTI_CLASS;

public:
	InputDriverX11();

	virtual bool create(const SystemWindow& systemWindow, uint32_t inputCategories);

	virtual int getDeviceCount();

	virtual Ref< IInputDevice > getDevice(int index);

	virtual UpdateResult update();

private:
	Display* m_display;
	Window m_window;
	int32_t m_opcode;
	RefArray< InputDeviceX11 > m_devices;
};

	}
}

#endif	// traktor_input_InputDriverX11_H
