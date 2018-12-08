#pragma once

#include <functional>
#include <list>
#include <map>
#include <vector>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "Core/Object.h"
#include "Ui/X11/TypesX11.h"

namespace traktor
{
    namespace ui
    {

struct WidgetData;

class Context : public Object
{
	T_RTTI_CLASS;

public:
    Context(Display* display, int screen, XIM xim);

	//! \brief Bind callback for specified event with given window target.
    void bind(WidgetData* widget, int32_t eventType, const std::function< void(XEvent& xe) >& fn);

	//! \brief Unbind all callbacks for specified window.
    void unbind(WidgetData* widget);

	//! \brief Defer callback until all dispatches has finished.
	void defer(const std::function< void() >& fn);

	//! \brief Push modal widget.
	void pushModal(WidgetData* widget);

	//! \brief Pop modal widget.
	void popModal();

	//! \brief Grab input to widget.
	void grab(WidgetData* widget);

	//! \brief Ungrab input.
	void ungrab(WidgetData* widget);

	//! \brief Set focus.
	void setFocus(WidgetData* widget);

	//! \brief Dispatch event to callbacks.
    void dispatch(XEvent& xe);

	//@{

	Display* getDisplay() const { return m_display; }

	int getScreen() const { return m_screen; }

	XIM getXIM() const { return m_xim; }

	//@}

private:
	struct Binding
	{
		WidgetData* widget;
		std::map< int32_t, std::function< void(XEvent& xe) > > fns;

		Binding()
		:	widget(nullptr)
		{
		}
	};

	Display* m_display;
	int m_screen;
	XIM m_xim;
    std::map< Window, Binding > m_bindings;
	std::list< std::function< void() > > m_deferred;
	std::vector< WidgetData* > m_modal;
	WidgetData* m_grabbed;
	WidgetData* m_focused;

    void dispatch(Window window, int32_t eventType, bool always, XEvent& xe);
};

    }
}
