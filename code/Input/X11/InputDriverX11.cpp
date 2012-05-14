#include "Core/Log/Log.h"
#include "Input/X11/InputDriverX11.h"

namespace traktor
{
    namespace input
    {

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputDriverX11", InputDriverX11, IInputDriver)

InputDriverX11::InputDriverX11()
:	m_display(0)
,	m_window(None)
{
}

bool InputDriverX11::create(const SystemWindow& systemWindow, uint32_t inputCategories)
{
	m_display = (Display*)systemWindow.display;
	m_window = (Window)systemWindow.window;
	return true;
}

int InputDriverX11::getDeviceCount()
{
	return 0;
}

Ref< IInputDevice > InputDriverX11::getDevice(int index)
{
	return 0;
}

InputDriverX11::UpdateResult InputDriverX11::update()
{
	XEvent evt;
	while (XCheckWindowEvent(m_display, m_window, KeyPressMask | KeyReleaseMask, &evt) == True)
	{
		T_DEBUG(L"Input event");
	}
	return UrOk;
}

    }
}
