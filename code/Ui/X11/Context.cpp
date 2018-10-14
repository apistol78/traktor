#include "Core/Assert.h"
#include "Ui/X11/Context.h"

namespace traktor
{
    namespace ui
    {

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Context", Context, Object)

Context::Context(Display* display, int screen, XIM xim)
:	m_display(display)
,	m_screen(screen)
,	m_xim(xim)
{
}

void Context::bind(WidgetData* widget, int32_t eventType, const std::function< void(XEvent& xe) >& fn)
{
	auto& b = m_bindings[widget->window];
	b.widget = widget;
	b.fns[eventType] = fn;
}

void Context::unbind(WidgetData* widget)
{
	T_FATAL_ASSERT(m_modal.empty());
	m_bindings.erase(widget->window);
}

void Context::pushModal(WidgetData* widget)
{
	m_modal.push(widget);
}

void Context::popModal()
{
	T_FATAL_ASSERT(!m_modal.empty());
	m_modal.pop();
}

void Context::dispatch(XEvent& xe)
{
    switch (xe.type)
    {
    case FocusIn:
        dispatch(xe.xfocus.window, FocusIn, true, xe);
        break;

    case FocusOut:
        dispatch(xe.xfocus.window, FocusOut, true, xe);
        break;

    case KeyPress:
        dispatch(xe.xkey.window, KeyPress, false, xe);
        break;

    case KeyRelease:
        dispatch(xe.xkey.window, KeyRelease, false, xe);
        break;

    case MotionNotify:
        dispatch(xe.xmotion.window, MotionNotify, false, xe);
        break;

	case EnterNotify:
		dispatch(xe.xcrossing.window, EnterNotify, false, xe);
		break;

	case LeaveNotify:
		dispatch(xe.xcrossing.window, LeaveNotify, false, xe);
		break;

    case ButtonPress:
        dispatch(xe.xbutton.window, ButtonPress, false, xe);
        break;

    case ButtonRelease:
        dispatch(xe.xbutton.window, ButtonRelease, false, xe);
        break;

    case ConfigureRequest:
        dispatch(xe.xconfigurerequest.window, ConfigureRequest, true, xe);
        break;

    case ConfigureNotify:
        dispatch(xe.xconfigure.window, ConfigureNotify, true, xe);
        break;
        
    case Expose:
        dispatch(xe.xexpose.window, Expose, true, xe);
        break;

    case ClientMessage:
        dispatch(xe.xclient.window, ClientMessage, true, xe);
        break;

    default:
        break;
    }
}

void Context::dispatch(Window window, int32_t eventType, bool always, XEvent& xe)
{
    auto b = m_bindings.find(window);
    if (b == m_bindings.end())
        return;

	T_FATAL_ASSERT(b->second.widget != nullptr);

	if (!always)
	{
		// If widget or parents is disabled then ignore event.
		for (const WidgetData* w = b->second.widget; w != nullptr; w = w->parent)
		{
			if (!w->enable)
				return;			
		}

		// If exclusive filtering is enabled then ensure widget is part of exclusive.
		if (!m_modal.empty())
		{
			bool p = false;
			for (const WidgetData* w = b->second.widget; w != nullptr; w = w->parent)
			{
				if (w == m_modal.top())
				{
					p = true;
					break;
				}
			}
			if (!p)
				return;
		}
	}

	auto d = b->second.fns.find(eventType);
	if (d == b->second.fns.end())
		return;

	std::function< void(XEvent& xe) > fn = d->second;
	fn(xe);
}

    }
}