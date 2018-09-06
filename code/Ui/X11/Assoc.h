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

    void bind(Drawable window, int32_t eventType, const std::function< void(XEvent& xe) >& fn);

    void unbind(Drawable window, int32_t eventType);

    void unbind(Drawable window);

    void dispatch(XEvent& xe);

private:
    typedef std::map< int32_t, std::function< void(XEvent& xe) > > binding_t;
    typedef std::map< Drawable, binding_t > bindings_t;

    bindings_t m_bindings;
    bool m_indispatch;

    Assoc();

    void dispatch(Drawable window, int32_t eventType, XEvent& xe);
};

    }
}
