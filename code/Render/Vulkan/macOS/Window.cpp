#include "Render/Vulkan/macOS/CGLWindow.h"
#include "Render/Vulkan/macOS/Window.h"

namespace traktor
{
	namespace render
	{

bool Window::create(int32_t width, int32_t height)
{
    DisplayMode displayMode;
    displayMode.width = width;
    displayMode.height = height;
    m_window = cglwCreateWindow(L"Traktor", displayMode, false);
	return true;
}

void Window::setTitle(const wchar_t* title)
{
}

void Window::setFullScreenStyle(int32_t /*width*/, int32_t /*height*/)
{
}

void Window::setWindowedStyle(int32_t width, int32_t height)
{
}

void Window::showCursor()
{
    cglwSetCursorVisible(m_window, true);
}

void Window::hideCursor()
{
    cglwSetCursorVisible(m_window, false);
}

void Window::show()
{
}

void Window::center()
{
}

bool Window::update(RenderEvent& outEvent)
{
    return cglwUpdateWindow(m_window, outEvent);
}

void* Window::getView() const
{
    return cglwGetWindowView(m_window);
}

	}
}
