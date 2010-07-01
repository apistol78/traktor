#ifndef traktor_render_CGLWindow_H
#define traktor_render_CGLWindow_H

#include "Core/Config.h"
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{
	
uint32_t cglwGetDisplayModeCount();

bool cglwGetDisplayMode(uint32_t index, DisplayMode& outDisplayMode);

bool cglwSetDisplayMode(const DisplayMode& displayMode);
	
void* cglwCreateWindow(const std::wstring& title, uint32_t width, uint32_t height, bool fullscreen);

void cglwDestroyWindow(void* windowHandle);

void cglwUpdateWindow(void* windowHandle);

void* cglwGetWindowView(void* windowHandle);
	
	}
}

#endif	// traktor_render_CGLWindow_H
