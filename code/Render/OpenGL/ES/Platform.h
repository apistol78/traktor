#pragma once

#include "Core/Config.h"
#if defined(_DEBUG)
#	include "Core/Assert.h"
#	include "Core/Io/StringOutputStream.h"
#	include "Core/Misc/TString.h"
#endif

#if !defined(T_OPENGL_ES)
#	define T_OPENGL_ES
#endif

#include <string>
#if defined(__IOS__)
#	include <OpenGLES/ES3/gl.h>
#	include <OpenGLES/ES3/glext.h>
#else
#	if defined(_WIN32)
#		define _WIN32_LEAN_AND_MEAN
#		include <windows.h>
#		include <tchar.h>
#	elif defined(__EMSCRIPTEN__)
#		include <emscripten.h>
#	endif
#	include <EGL/egl.h>
#	include <GLES3/gl31.h>
#	define T_OPENGL_ES_HAVE_EGL
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

#if defined(T_OPENGL_ES_HAVE_EGL)
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
			glErrorMsg << L"OpenGL error, " << uint32_t(glError) << L" (" << getOpenGLErrorString(glError) << L") " << mbstows(T_FILE_LINE); \
			T_ASSERT_M(glError == GL_NO_ERROR, glErrorMsg.str().c_str()); \
		} \
	}
#else
#	define T_OGL_SAFE(CALL) { (CALL); }
#endif

//@}

