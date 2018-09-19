#pragma once

#include <functional>
#include <map>
#include <X11/Xlib.h>

namespace traktor
{
    namespace ui
    {

class Assoc 
{
public:
    static Assoc& getInstance();

    void bind(Window window, int32_t eventType, const std::function< void(XEvent& xe) >& fn);

    void unbind(Window window, int32_t eventType);

    void unbind(Window window);

    void dispatch(Display* display, XEvent& xe);

	void setEnable(Window window, bool enable);

private:
	struct Binding
	{
		bool enable;
		std::map< int32_t, std::function< void(XEvent& xe) > > dispatch;

		Binding()
		:	enable(true)
		{
		}
	};

    typedef std::map< Window, Binding > bindings_t;

    bindings_t m_bindings;

    void dispatch(Display* display, Window window, int32_t eventType, bool always, XEvent& xe);

	bool allowed(Display* display, Window window);
};

    }
}
