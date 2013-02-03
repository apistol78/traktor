#ifndef traktor_render_Window_H
#define traktor_render_Window_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>
#include "Core/Object.h"
#include "Render/Types.h"

namespace traktor
{
    namespace render
    {

class Window : public Object
{
public:
    Window(::Display* display);

    virtual ~Window();

    bool create(int32_t width, int32_t height);

    void setTitle(const wchar_t* title);

    void setFullScreenStyle(int32_t width, int32_t height);

    void setWindowedStyle(int32_t width, int32_t height);

    void show();

    bool update(RenderEvent& outEvent);

    int32_t getWidth() const;

    int32_t getHeight() const;

    ::Display* getDisplay() const { return m_display; }

    ::Window getWindow() const { return m_window; }

private:
    ::Display* m_display;
    ::Window m_window;
    int m_screen;
    XRRScreenConfiguration* m_originalConfig;
    int m_originalSizeIndex;
    int m_originalRate;
    Rotation m_originalRotation;
};

    }
}

#endif  // traktor_render_Window_H
