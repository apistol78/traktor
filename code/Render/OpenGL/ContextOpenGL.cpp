#include "Render/OpenGL/ContextOpenGL.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{

#if defined(_WIN32)

ContextOpenGL::ContextOpenGL(HWND hWnd, HDC hDC, HGLRC hRC)
:	m_hWnd(hWnd)
,	m_hDC(hDC)
,	m_hRC(hRC)
,	m_count(0)
,	m_ownerThreadId(0)

#elif defined(__APPLE__)

ContextOpenGL::ContextOpenGL(WindowRef window, ControlRef control, AGLContext context)
:	m_window(window)
,	m_control(control)
,	m_context(context)

#else	// LINUX

ContextOpenGL::ContextOpenGL(Display* display, Window window, GLXContext context)
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
#else	// LINUX
	T_ASSERT (!m_context);
#endif
}

bool ContextOpenGL::enter()
{
#if defined(_WIN32)

	uint32_t threadId = GetCurrentThreadId();

	if (m_count++ > 0)
	{
		T_ASSERT (m_ownerThreadId == threadId);
		return true;
	}

	T_ASSERT (m_ownerThreadId == 0);
	if (!wglMakeCurrent(m_hDC, m_hRC))
		return false;

	m_ownerThreadId = threadId;

#elif defined(__APPLE__)

	if (aglSetCurrentContext(m_context) != GL_TRUE)
		return false;
	
	HIRect controlRect;
	HIViewGetBounds(m_control, &controlRect);
	
	HIViewRef root = HIViewGetRoot(HIViewGetWindow(m_control));
	
	HIRect windowRect;
	HIViewGetBounds(root, &windowRect);
	HIViewConvertRect(&controlRect, m_control, root);

	GLint params[4] = { 0, 0, 0, 0 };
	if (HIViewIsVisible(m_control))
	{
		params[0] = controlRect.origin.x;
		params[1] = windowRect.size.height - (controlRect.origin.y + controlRect.size.height);
		params[2] = controlRect.size.width;
		params[3] = controlRect.size.height;
	}
	
	aglSetInteger(m_context, AGL_BUFFER_RECT, params);
	aglEnable(m_context, AGL_BUFFER_RECT);

#else	// LINUX

	if (m_window)
		glXMakeCurrent(
			m_display,
			m_window,
			m_context
		);

#endif

	return true;
}

void ContextOpenGL::leave()
{
#if defined(_WIN32)
	T_ASSERT (m_count > 0);
	T_ASSERT (m_ownerThreadId == GetCurrentThreadId());

	if (--m_count == 0)
	{
		wglMakeCurrent(m_hDC, NULL);
		m_ownerThreadId = 0;
	}

#elif defined(__APPLE__)
#else	// LINUX
#endif
}

void ContextOpenGL::share(ContextOpenGL* context)
{
#if defined(_WIN32)
	wglShareLists(context->m_hRC, m_hRC);
	wglShareLists(m_hRC, context->m_hRC);
#elif defined(__APPLE__)
#else	// LINUX
#endif
}

void ContextOpenGL::update()
{
#if defined(__APPLE__)
	aglUpdateContext(m_context);
#endif
}

void ContextOpenGL::swapBuffers()
{
#if defined(_WIN32)
	SwapBuffers(m_hDC);
#elif defined(__APPLE__)
	aglSwapBuffers(m_context);
#else	// LINUX
	glXSwapBuffers(m_display, m_window);
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
	{
		aglDestroyContext(m_context);
		m_context = 0;
	}

#else	// LINUX

	if (m_context)
	{
		glXDestroyContext(m_display, m_context);
		m_context = 0;
	}

#endif
}

void ContextOpenGL::deleteResource(DeleteCallback* callback)
{
	m_deleteResources.push_back(callback);
}

void ContextOpenGL::deleteResources()
{
	for (std::vector< DeleteCallback* >::iterator i = m_deleteResources.begin(); i != m_deleteResources.end(); ++i)
		(*i)->deleteResource();
	m_deleteResources.resize(0);
}

	}
}
