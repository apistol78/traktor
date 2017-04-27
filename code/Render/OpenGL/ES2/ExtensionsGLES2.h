/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ExtensionsGLES2_H
#define traktor_render_ExtensionsGLES2_H

#include "Render/OpenGL/Platform.h"

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

#if defined(__ANDROID__)
extern PFNGLDISCARDFRAMEBUFFEREXTPROC s_glDiscardFramebufferEXT;
#endif

void initializeExtensions();

	}
}

#endif	// traktor_render_ExtensionsGLES2_H
