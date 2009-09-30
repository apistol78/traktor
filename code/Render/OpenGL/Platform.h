#ifndef traktor_render_Platform_H
#define traktor_render_Platform_H

#include "Core/Config.h"

#if defined(TARGET_OS_IPHONE)
#	if !defined(T_OPENGL_ES2)
#		define T_OPENGL_ES2
#	endif
#endif

#include <string>
#if defined(T_OPENGL_STD)
#	if defined(_WIN32)
#		define _WIN32_LEAN_AND_MEAN
#		include <windows.h>
#		include <tchar.h>
#		include <gl/gl.h>
#		include <gl/glu.h>
#		include <gl/glext.h>
#	elif defined(__APPLE__)
#		include <Carbon/Carbon.h>
#		include <AGL/agl.h>
#		include <OpenGL/gl.h>
#		include <OpenGL/glu.h>
#		include <OpenGL/glext.h>
#	else	// LINUX
#		if !defined(GLX_GLXEXT_LEGACY)
#			define GLX_GLXEXT_LEGACY
#		endif
#		include <GL/gl.h>
#		include <GL/glu.h>
#		include <GL/glext.h>
#		include <GL/glx.h>
#	endif
#elif defined(T_OPENGL_ES2)
#	if !defined(TARGET_OS_IPHONE)
#		if defined(_WIN32)
#			define _WIN32_LEAN_AND_MEAN
#			include <windows.h>
#			include <tchar.h>
#		endif
#		include <EGL/egl.h>
#		include <GLES2/gl2.h>
#		include <GLES2/gl2ext.h>
#		if defined(_WIN32)
#			include <GLES2/gl2amdext.h>
#		endif
#		define T_OPENGL_ES2_HAVE_EGL
#	else
#		include <OpenGLES/ES2/gl.h>
#		include <OpenGLES/ES2/glext.h>
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
#	include "Core/Assert.h"
#	include "Core/Io/StringOutputStream.h"
#	define T_OGL_SAFE(CALL) \
	{ \
		(CALL); \
		GLenum glError = glGetError(); \
		if (glError != GL_NO_ERROR) \
		{ \
			StringOutputStream glErrorMsg; \
			glErrorMsg << L"OpenGL error, " << uint32_t(glError) << L" (" << getOpenGLErrorString(glError) << L")"; \
			T_ASSERT_M(glError == GL_NO_ERROR, glErrorMsg.str()); \
		} \
	}
#else
#	define T_OGL_SAFE(CALL) { (CALL); }
#endif

//@}

#endif	// traktor_render_Platform_H
