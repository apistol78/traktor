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
#endif

#if TARGET_OS_IPHONE || TARGET_OS_IPAD
#	if !defined(T_OPENGL_ES2)
#		define T_OPENGL_ES2
#	endif
#elif TARGET_OS_MAC && !defined(T_OPENGL_ES2)
#	if !defined(T_OPENGL_STD)
#		define T_OPENGL_STD
#	endif
#endif

#include <string>
#if defined(T_OPENGL_STD)
#	if defined(_WIN32)
#		define _WIN32_LEAN_AND_MEAN
#		include <windows.h>
#		include <tchar.h>
#		include <GL/glew.h>
#		include <GL/wglew.h>
#	elif defined(__APPLE__)
#		include <OpenGL/gl3.h>
#		include <OpenGL/gl3ext.h>
#	else	// LINUX
#		if !defined(GLX_GLXEXT_LEGACY)
#			define GLX_GLXEXT_LEGACY
#		endif
#		include <GL/glew.h>
#		include <GL/glxew.h>
#	endif
#elif defined(T_OPENGL_ES2)
#	if TARGET_OS_IPHONE || TARGET_OS_IPAD
#		include <OpenGLES/ES2/gl.h>
#		include <OpenGLES/ES2/glext.h>
#	elif __PNACL__
#		include <GLES2/gl2.h>
#		include <GLES2/gl2ext.h>
#	else
#		if defined(_WIN32)
#			define _WIN32_LEAN_AND_MEAN
#			include <windows.h>
#			include <tchar.h>
#		elif defined(__EMSCRIPTEN__)
#			include <emscripten.h>
#		endif
#		include <EGL/egl.h>
#		include <GLES2/gl2.h>
#		include <GLES2/gl2ext.h>
#		define T_OPENGL_ES2_HAVE_EGL
#	endif
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

#if defined(T_OPENGL_ES2_HAVE_EGL)
std::wstring getEGLErrorString(EGLint error);
#endif

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
			glErrorMsg << L"OpenGL error, " << uint32_t(glError) << L" (" << getOpenGLErrorString(glError) << L") " << T_FILE_LINE; \
			T_ASSERT_M(glError == GL_NO_ERROR, glErrorMsg.str()); \
		} \
	}
#else
#	define T_OGL_SAFE(CALL) { (CALL); }
#endif

//@}

#endif	// traktor_render_Platform_H
