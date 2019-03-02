#pragma once

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

