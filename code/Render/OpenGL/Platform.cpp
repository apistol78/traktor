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
	case GL_STACK_OVERFLOW:
		return L"GL_STACK_OVERFLOW";
	case GL_STACK_UNDERFLOW:
		return L"GL_STACK_UNDERFLOW";
	case GL_OUT_OF_MEMORY:
		return L"GL_OUT_OF_MEMORY";
	case GL_INVALID_FRAMEBUFFER_OPERATION_EXT:
		return L"GL_INVALID_FRAMEBUFFER_OPERATION";
	}
	return L"Unknown GL error";
}

	}
}
