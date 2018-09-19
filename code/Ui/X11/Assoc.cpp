#include "Core/Assert.h"
#include "Ui/X11/Assoc.h"

namespace traktor
{
    namespace ui
    {

Assoc& Assoc::getInstance()
{
    static Assoc s_instance;
    return s_instance;
}

void Assoc::bind(Window window, int32_t eventType, const std::function< void(XEvent& xe) >& fn)
{
    auto& b = m_bindings[window];
	b.dispatch[eventType] = fn;
}

void Assoc::unbind(Window window, int32_t eventType)
{
	auto& b = m_bindings[window];
    b.dispatch.erase(eventType);
}

void Assoc::unbind(Window window)
{
	auto& b = m_bindings[window];
    b.dispatch.clear();
}

void Assoc::dispatch(Display* display, XEvent& xe)
{
    switch (xe.type)
    {
    case FocusIn:
        dispatch(display, xe.xfocus.window, FocusIn, true, xe);
        break;

    case FocusOut:
        dispatch(display, xe.xfocus.window, FocusOut, true, xe);
        break;

    case KeyPress:
        dispatch(display, xe.xkey.window, KeyPress, false, xe);
        break;

    case KeyRelease:
        dispatch(display, xe.xkey.window, KeyRelease, false, xe);
        break;

    case MotionNotify:
        dispatch(display, xe.xmotion.window, MotionNotify, false, xe);
        break;

    case ButtonPress:
        dispatch(display, xe.xbutton.window, ButtonPress, false, xe);
        break;

    case ButtonRelease:
        dispatch(display, xe.xbutton.window, ButtonRelease, false, xe);
        break;

    case ConfigureRequest:
        dispatch(display, xe.xconfigurerequest.window, ConfigureRequest, true, xe);
        break;

    case ConfigureNotify:
        dispatch(display, xe.xconfigure.window, ConfigureNotify, true, xe);
        break;
        
    case Expose:
        dispatch(display, xe.xexpose.window, Expose, true, xe);
        break;

    case ClientMessage:
        dispatch(display, xe.xclient.window, ClientMessage, true, xe);
        break;

    default:
        break;
    }
}

void Assoc::setEnable(Window window, bool enable)
{
    auto b = m_bindings.find(window);
    if (b != m_bindings.end())
        b->second.enable = enable;
}

void Assoc::dispatch(Display* display, Window window, int32_t eventType, bool always, XEvent& xe)
{
    auto b = m_bindings.find(window);
    if (b == m_bindings.end())
        return;

	if (!always && !allowed(display, window))
		return;

	auto d = b->second.dispatch.find(eventType);
	if (d == b->second.dispatch.end())
		return;

    std::function< void(XEvent& xe) > fn = d->second;
    fn(xe);
}

bool Assoc::allowed(Display* display, Window window)
{
    auto b = m_bindings.find(window);
    if (b == m_bindings.end())
        return true;

	if (!b->second.enable)
		return false;

	Window root = None, parent = None;
	Window* children = nullptr;
	unsigned int nchildren = 0;

	XQueryTree(display, window, &root, &parent, &children, &nchildren);
	if (children != nullptr)
		XFree(children);

	if (parent == root || parent == None || parent == DefaultRootWindow(display))
		return true;

	return allowed(display, parent);
}

    }
}