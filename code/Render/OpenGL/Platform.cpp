#include "Render/OpenGL/Platform.h"

namespace traktor
{
	namespace render
	{

std::wstring getOpenGLErrorString(GLenum glError)
{
	switch (glError)
	{
	case GL_NO_ERROR:
		return L"GL_NO_ERROR";
	case GL_INVALID_ENUM:
		return L"GL_INVALID_ENUM";
	case GL_INVALID_VALUE:
		return L"GL_INVALID_VALUE";
	case GL_INVALID_OPERATION:
		return L"GL_INVALID_OPERATION";
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		return L"GL_INVALID_FRAMEBUFFER_OPERATION";
	case GL_OUT_OF_MEMORY:
		return L"GL_OUT_OF_MEMORY";
	}
	return L"Unknown GL error";
}

#if defined(T_OPENGL_ES2_HAVE_EGL)
std::wstring getEGLErrorString(EGLint error)
{
	switch (error)
	{
	case EGL_SUCCESS:
		return L"EGL_SUCCESS";
	case EGL_NOT_INITIALIZED:
		return L"EGL_NOT_INITIALIZED";
	case EGL_BAD_ACCESS:
		return L"EGL_BAD_ACCESS";
	case EGL_BAD_ALLOC:
		return L"EGL_BAD_ALLOC";
	case EGL_BAD_ATTRIBUTE:
		return L"EGL_BAD_ATTRIBUTE";
	case EGL_BAD_CONFIG:
		return L"EGL_BAD_CONFIG";
	case EGL_BAD_CONTEXT:
		return L"EGL_BAD_CONTEXT";
	case EGL_BAD_CURRENT_SURFACE:
		return L"EGL_BAD_CURRENT_SURFACE";
	case EGL_BAD_DISPLAY:
		return L"EGL_BAD_DISPLAY";
	case EGL_BAD_MATCH:
		return L"EGL_BAD_MATCH";
	case EGL_BAD_NATIVE_PIXMAP:
		return L"EGL_BAD_NATIVE_PIXMAP";
	case EGL_BAD_NATIVE_WINDOW:
		return L"EGL_BAD_NATIVE_WINDOW";
	case EGL_BAD_PARAMETER:
		return L"EGL_BAD_PARAMETER";
	case EGL_BAD_SURFACE:
		return L"EGL_BAD_SURFACE";
	case EGL_CONTEXT_LOST:
		return L"EGL_CONTEXT_LOST";
	}
	return L"Unknown EGL error";
}
#endif

	}
}
