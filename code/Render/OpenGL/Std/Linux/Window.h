#ifndef traktor_render_Window_H
#define traktor_render_Window_H

#include <X11/Xlib.h>
#include "Core/Object.h"

namespace traktor
{
    namespace render
    {

class Window : public Object
{
public:
    Window();

    virtual ~Window();

    bool create();

    bool update();

    ::Display* getDisplay() const { return m_display; }

    ::Window getWindow() const { return m_window; }

private:
    ::Display* m_display;
    ::Window m_window;
};

    }
}

#endif  // traktor_render_Window_H
