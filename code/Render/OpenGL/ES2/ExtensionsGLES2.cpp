#if defined(__ANDROID__)
#	include <dlfcn.h>
#endif
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Render/OpenGL/ES2/ExtensionsGLES2.h"

namespace traktor
{
	namespace render
	{

#if defined(GL_OES_vertex_array_object) && !defined(_WIN32) && !defined(__IOS__)
PFNGLBINDVERTEXARRAYOESPROC g_glBindVertexArrayOES = 0;
PFNGLDELETEVERTEXARRAYSOESPROC g_glDeleteVertexArraysOES = 0;
PFNGLGENVERTEXARRAYSOESPROC g_glGenVertexArraysOES = 0;
#endif

#if defined(__ANDROID__)
PFNGLDISCARDFRAMEBUFFEREXTPROC s_glDiscardFramebufferEXT = 0;
#endif

void initializeExtensions()
{
	const char* supported = (const char*)glGetString(GL_EXTENSIONS);
	while (supported && *supported)
	{
		const char* end = supported;
		while (*end && *end != ' ')
		{
			if ((++end - supported) >= 200)
				break;
		}

		int32_t length = end - supported;

		char tmp[1024];
		strncpy(tmp, supported, length);
		tmp[length] = 0;

		log::info << mbstows(tmp) << Endl;
			
		supported = end;
		while (*supported == ' ')
		{
			if ((++supported - end) >= 10)
				break;
		}
	}

#if defined(__ANDROID__)
	void* libhandle = dlopen("libGLESv2.so", RTLD_LAZY);
	if (libhandle)
	{
#	if defined(GL_OES_vertex_array_object)	
		g_glBindVertexArrayOES = (PFNGLBINDVERTEXARRAYOESPROC)dlsym(libhandle, "glBindVertexArrayOES");
#		if defined(_DEBUG)
		if (g_glBindVertexArrayOES)
			log::info << L"glBindVertexArrayOES found!" << Endl;
		else
			log::info << L"glBindVertexArrayOES NOT found!" << Endl;
#		endif

		g_glDeleteVertexArraysOES = (PFNGLDELETEVERTEXARRAYSOESPROC)dlsym(libhandle, "glDeleteVertexArraysOES");
#		if defined(_DEBUG)
		if (g_glDeleteVertexArraysOES)
			log::info << L"glDeleteVertexArraysOES found!" << Endl;
		else
			log::info << L"glDeleteVertexArraysOES NOT found!" << Endl;
#		endif

		g_glGenVertexArraysOES = (PFNGLGENVERTEXARRAYSOESPROC)dlsym(libhandle, "glGenVertexArraysOES");
#		if defined(_DEBUG)
		if (g_glGenVertexArraysOES)
			log::info << L"glGenVertexArraysOES found!" << Endl;
		else
			log::info << L"glGenVertexArraysOES NOT found!" << Endl;
#		endif
#	endif

		s_glDiscardFramebufferEXT = (PFNGLDISCARDFRAMEBUFFEREXTPROC)dlsym(RTLD_DEFAULT, "glDiscardFramebufferEXT");
		if (g_glGenVertexArraysOES)
			log::info << L"glDiscardFramebufferEXT found!" << Endl;
		else
			log::info << L"glDiscardFramebufferEXT NOT found!" << Endl;
	}
#endif
}

	}
}
