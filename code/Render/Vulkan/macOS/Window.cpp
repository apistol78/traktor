/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Vulkan/macOS/CGLWindow.h"
#include "Render/Vulkan/macOS/Window.h"

namespace traktor
{
	namespace render
	{

Window::~Window()
{
    if (m_window)
        cglwDestroyWindow(m_window);
}

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

bool Window::isFullScreen() const
{
    return cglwIsFullscreen(m_window);
}

bool Window::isActive() const
{
    return cglwIsActive(m_window);
}

void* Window::getView() const
{
    return cglwGetWindowView(m_window);
}

	}
}
