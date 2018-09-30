/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_Platform_H
#define traktor_render_Platform_H

#include "Core/Config.h"
#if defined(_DEBUG)
#	include "Core/Assert.h"
#	include "Core/Io/StringOutputStream.h"
#	include "Core/Misc/TString.h"
#endif

#if !defined(T_OPENGL_STD)
#	define T_OPENGL_STD
#endif

#include <string>
#if defined(_WIN32)
#	define _WIN32_LEAN_AND_MEAN
#	include <windows.h>
#	include <tchar.h>
#	include <GL/glew.h>
#	include <GL/wglew.h>
#elif defined(__APPLE__)
#	include <OpenGL/gl3.h>
#	include <OpenGL/gl3ext.h>
#else	// LINUX
#	if !defined(GLX_GLXEXT_LEGACY)
#		define GLX_GLXEXT_LEGACY
#	endif
#	include <GL/glew.h>
#	include <GL/glxew.h>
#endif

/*! \ingroup OGL */
//@{

#define T_OGL_USAGE_INDEX(usage, index) (((usage) << 2) | (index))
#define T_OGL_MAX_USAGE (6)
#define T_OGL_MAX_INDEX (3)
#define T_OGL_MAX_USAGE_INDEX (((T_OGL_MAX_USAGE) << 2) | T_OGL_MAX_INDEX)

namespace traktor
{
	namespace render
	{

std::wstring getOpenGLErrorString(GLenum glError);

	}
}

#if defined(_DEBUG)
#	define T_OGL_SAFE(CALL) \
	{ \
		(CALL); \
		GLenum glError = glGetError(); \
		if (glError != GL_NO_ERROR) \
		{ \
			StringOutputStream glErrorMsg; \
			glErrorMsg << L"OpenGL error, " << uint32_t(glError) << L" (" << getOpenGLErrorString(glError) << L") " << mbstows(T_FILE_LINE); \
			T_ASSERT_M(glError == GL_NO_ERROR, glErrorMsg.str().c_str()); \
		} \
	}
#else
#	define T_OGL_SAFE(CALL) { (CALL); }
#endif

//@}

#endif	// traktor_render_Platform_H
