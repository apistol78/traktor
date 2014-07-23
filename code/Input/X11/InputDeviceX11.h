#ifndef traktor_input_InputDeviceX11_H
#define traktor_input_InputDeviceX11_H

#include <X11/Xlib.h>
#include "Input/IInputDevice.h"

namespace traktor
{
	namespace input
	{

class InputDeviceX11 : public IInputDevice
{
	T_RTTI_CLASS;

public:
	virtual void consumeEvent(XEvent& evt) = 0;

	virtual void setFocus(bool focus) = 0;
};

	}
}

#endif	// traktor_input_InputDeviceX11_H
