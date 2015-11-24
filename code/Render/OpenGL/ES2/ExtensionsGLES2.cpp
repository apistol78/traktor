#if defined(__ANDROID__)
#	include <dlfcn.h>
#endif
#include "Core/Log/Log.h"
#include "Render/OpenGL/ES2/ExtensionsGLES2.h"

namespace traktor
{
	namespace render
	{

#if defined(GL_OES_vertex_array_object)
PFNGLBINDVERTEXARRAYOESPROC g_glBindVertexArrayOES = 0;
PFNGLDELETEVERTEXARRAYSOESPROC g_glDeleteVertexArraysOES = 0;
PFNGLGENVERTEXARRAYSOESPROC g_glGenVertexArraysOES = 0;
#endif

void initializeExtensions()
{
#if defined(__IOS__)
#	if defined(GL_OES_vertex_array_object)
	g_glBindVertexArrayOES = glBindVertexArrayOES;
	g_glDeleteVertexArraysOES = glDeleteVertexArraysOES;
	g_glGenVertexArraysOES = glGenVertexArraysOES;
#	endif
#elif defined(__ANDROID__)
#	if defined(GL_OES_vertex_array_object)	
	void* libhandle = dlopen("libGLESv2.so", RTLD_LAZY);
	if (libhandle)
	{
		g_glBindVertexArrayOES = (PFNGLBINDVERTEXARRAYOESPROC)dlsym(libhandle, "glBindVertexArrayOES");
		if (g_glBindVertexArrayOES)
			log::info << L"glBindVertexArrayOES found!" << Endl;
		else
			log::info << L"glBindVertexArrayOES NOT found!" << Endl;

		g_glDeleteVertexArraysOES = (PFNGLDELETEVERTEXARRAYSOESPROC)dlsym(libhandle, "glDeleteVertexArraysOES");
		if (g_glDeleteVertexArraysOES)
			log::info << L"glDeleteVertexArraysOES found!" << Endl;
		else
			log::info << L"glDeleteVertexArraysOES NOT found!" << Endl;

		g_glGenVertexArraysOES = (PFNGLGENVERTEXARRAYSOESPROC)dlsym(libhandle, "glGenVertexArraysOES");
		if (g_glGenVertexArraysOES)
			log::info << L"glGenVertexArraysOES found!" << Endl;
		else
			log::info << L"glGenVertexArraysOES NOT found!" << Endl;
	}
#	endif
#endif
}

	}
}
