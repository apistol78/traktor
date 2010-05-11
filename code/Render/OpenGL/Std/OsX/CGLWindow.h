#ifndef traktor_render_CGLWindow_H
#define traktor_render_CGLWindow_H

#include "Core/Config.h"

namespace traktor
{
	namespace render
	{
	
void* cglwCreateWindow(const std::wstring& title, uint32_t width, uint32_t height, bool fullscreen);

void cglwDestroyWindow(void* windowHandle);

void cglwUpdateWindow(void* windowHandle);

void* cglwGetWindowView(void* windowHandle);
	
	}
}

#endif	// traktor_render_CGLWindow_H
