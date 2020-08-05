#pragma once

#include "Render/OpenGL/ES/Platform.h"

namespace traktor
{
	namespace render
	{

#if defined(GL_OES_vertex_array_object)
#	if defined(__IOS__)
#		define g_glBindVertexArrayOES glBindVertexArrayOES
#		define g_glDeleteVertexArraysOES glDeleteVertexArraysOES
#		define g_glGenVertexArraysOES glGenVertexArraysOES
#	else
extern PFNGLBINDVERTEXARRAYOESPROC g_glBindVertexArrayOES;
extern PFNGLDELETEVERTEXARRAYSOESPROC g_glDeleteVertexArraysOES;
extern PFNGLGENVERTEXARRAYSOESPROC g_glGenVertexArraysOES;
#	endif
#endif

#if defined(__ANDROID__) || defined(__LINUX__)
extern PFNGLDISCARDFRAMEBUFFEREXTPROC s_glDiscardFramebufferEXT;
extern PFNGLDRAWELEMENTSINSTANCEDEXTPROC s_glDrawElementsInstancedEXT;
extern PFNGLDRAWARRAYSINSTANCEDEXTPROC s_glDrawArraysInstancedEXT;
#endif

void initializeExtensions();

	}
}

