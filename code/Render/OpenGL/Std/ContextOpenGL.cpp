#include "Core/RefArray.h"
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Render/OpenGL/Std/ContextOpenGL.h"

#if defined(__APPLE__)
#	include "Render/OpenGL/Std/OsX/CGLWrapper.h"
#endif

namespace traktor
{
	namespace render
	{
		namespace
		{

typedef RefArray< ContextOpenGL > context_stack_t;

#if !defined(__APPLE__)
void debugCallbackARB(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, GLvoid *userParam)
{
#	if !defined(_DEBUG)
	if (severity != GL_DEBUG_SEVERITY_MEDIUM_ARB && severity != GL_DEBUG_SEVERITY_HIGH_ARB)
		return;
#	endif

	const wchar_t* s = L"unknown";

	if (severity == GL_DEBUG_SEVERITY_LOW_ARB)
		s = L"low";
	else if (severity == GL_DEBUG_SEVERITY_MEDIUM_ARB)
		s = L"medium";
	else if (severity == GL_DEBUG_SEVERITY_HIGH_ARB)
		s = L"high";

	if (message)
		log::info << L"OpenGL (" << s << L"): " << mbstows(message) << Endl;
	else
		log::info << L"OpenGL (" << s << L"): <empty>" << Endl;
}
#endif

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ContextOpenGL", ContextOpenGL, Object)

ThreadLocal ContextOpenGL::ms_contextStack;

#if defined(_WIN32)
ContextOpenGL::ContextOpenGL(HWND hWnd, HDC hDC, HGLRC hRC)
:	m_hWnd(hWnd)
,	m_hDC(hDC)
,	m_hRC(hRC)
#elif defined(__APPLE__)
ContextOpenGL::ContextOpenGL(void* context)
:	m_context(context)
#elif defined(__LINUX__)
ContextOpenGL::ContextOpenGL(::Display* display, ::Window window, GLXContext context)
:	m_display(display)
,	m_window(window)
,	m_context(context)
#endif
{
}

ContextOpenGL::~ContextOpenGL()
{
#if defined(_WIN32)
	T_ASSERT (!m_hRC);
#elif defined(__APPLE__)
	T_ASSERT (!m_context);
#elif defined(__LINUX__)
	T_ASSERT (!m_context);
#endif
}

void ContextOpenGL::destroy()
{
#if defined(_WIN32)
	if (m_hRC)
	{
		wglMakeCurrent(0, 0);
		wglDeleteContext(m_hRC);
		ReleaseDC(m_hWnd, m_hDC);
		m_hWnd = 0;
		m_hDC = 0;
		m_hRC = 0;
	}
#elif defined(__APPLE__)
	if (m_context)
		m_context = 0;
#elif defined(__LINUX__)
	if (m_context)
	{
		glXDestroyContext(m_display, m_context);
		m_context = 0;
	}
#endif
}

bool ContextOpenGL::enter()
{
	if (!m_lock.wait())
		return false;

#if defined(_WIN32)
	if (!wglMakeCurrent(m_hDC, m_hRC))
		return false;
#elif defined(__APPLE__)
	if (!cglwMakeCurrent(m_context))
		return false;
#elif defined(__LINUX__)
	if (!glXMakeCurrent(
		m_display,
		m_window,
		m_context
	))
		return false;
#endif

	context_stack_t* stack = static_cast< context_stack_t* >(ms_contextStack.get());
	if (!stack)
	{
		stack = new context_stack_t();
		ms_contextStack.set(stack);
	}

// #if !defined(__APPLE__)
// 	if (glDebugMessageCallbackARB)
// 	{
// 		glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
// 		glDebugMessageCallbackARB((GLDEBUGPROCARB)&debugCallbackARB, 0);
// #	if defined(_WIN32)
// #		if defined(_DEBUG)
// 		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
// #		else
// 		glEnable(GL_DEBUG_OUTPUT);
// #		endif
// #	endif
// 	}
// #endif

	// Flush GL error stack.
	while (glGetError() != GL_NO_ERROR)
		;

	stack->push_back(this);
	return true;
}

void ContextOpenGL::leave()
{
	context_stack_t* stack = static_cast< context_stack_t* >(ms_contextStack.get());
	bool result = true;

	T_ASSERT (stack);
	T_ASSERT (!stack->empty());
	T_ASSERT (stack->back() == this);

	stack->pop_back();

#if defined(_WIN32)
	if (!stack->empty())
	{
		wglMakeCurrent(
			stack->back()->m_hDC,
			stack->back()->m_hRC
		);
	}
	else
		wglMakeCurrent(m_hDC, NULL);
#elif defined(__APPLE__)
	if (!stack->empty())
		cglwMakeCurrent(stack->back()->m_context);
	else
		cglwMakeCurrent(0);
#elif defined(__LINUX__)
	if (!stack->empty())
	{
		result = (glXMakeContextCurrent(
			stack->back()->m_display,
			stack->back()->m_window,
			stack->back()->m_window,
			stack->back()->m_context
		) == True);
	}
	else
	{
		result = (glXMakeContextCurrent(
			m_display,
			None,
			None,
			NULL
		) == True);
	}
#endif

	m_lock.release();
}

	}
}
