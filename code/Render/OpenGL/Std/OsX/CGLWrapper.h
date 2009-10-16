#ifndef traktor_render_CGLWrapper_H
#define traktor_render_CGLWrapper_H

namespace traktor
{
	namespace render
	{

void* cglwCreateContext(void* nativeWindowHandle, void* sharedContext);

void cglwUpdate(void* context);

bool cglwMakeCurrent(void* context);

void cglwSwapBuffers(void* context);

	}
}

#endif	// traktor_render_CGLWrapper_H
