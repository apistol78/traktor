#pragma once

#if defined(__LINUX__)
#	include <X11/Xlib.h>
#endif
#include "Core/Object.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/ThreadLocal.h"
#include "Render/OpenGL/Std/Platform.h"
#include "Render/OpenGL/Std/TypesOpenGL.h"

namespace traktor
{
	namespace render
	{

/*! \brief OpenGL context.
 * \ingroup OGL
 */
class ContextOpenGL : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Scoped enter/leave helper.
	 * \ingroup OGL
	 */
	struct Scope
	{
		ContextOpenGL* m_context;

		Scope(ContextOpenGL* context)
		:	m_context(context)
		{
			bool result = m_context->enter();
			T_FATAL_ASSERT_M (result, L"Unable to set OpenGL context!");
		}

		~Scope()
		{
			m_context->leave();
		}
	};

#if defined(_WIN32)
	ContextOpenGL(HWND hWnd, HDC hDC, HGLRC hRC);
#elif defined(__APPLE__)
	ContextOpenGL(void* context);
#elif defined(__LINUX__)
	ContextOpenGL(::Display* display, ::Window window, GLXContext context);
#endif

	virtual ~ContextOpenGL();

	virtual void destroy();

	bool enter();

	void leave();

#if defined(_WIN32)
	HGLRC getGLRC() const { return m_hRC; }
#elif defined(__APPLE__)
	void* getGLContext() const { return m_context; }
#elif defined(__LINUX__)
	GLXContext getGLXContext() const { return m_context; }
#endif

protected:
#if defined(_WIN32)
	HWND m_hWnd;
	HDC m_hDC;
	HGLRC m_hRC;
#elif defined(__APPLE__)
	void* m_context;
#elif defined(__LINUX__)
	::Display* m_display;
	::Window m_window;
	GLXContext m_context;
#endif

	static ThreadLocal ms_contextStack;
	Semaphore m_lock;
};

	}
}

