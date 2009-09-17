#ifndef traktor_render_ContextOpenGL_H
#define traktor_render_ContextOpenGL_H

#include <map>
#include "Core/Object.h"
#include "Core/Thread/ThreadLocal.h"
#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/IContext.h"

namespace traktor
{
	namespace render
	{

/*! \brief OpenGL context.
 * \ingroup OGL
 */
class ContextOpenGL : public IContext
{
	T_RTTI_CLASS(ContextOpenGL)

public:
#if defined(_WIN32)
	ContextOpenGL(HWND hWnd, HDC hDC, HGLRC hRC);
#elif defined(__APPLE__)
	ContextOpenGL(WindowRef window, ControlRef control, AGLContext context);
#else	// LINUX
	ContextOpenGL(Display* display, Window window, GLXContext context);
#endif

	virtual ~ContextOpenGL();

	void share(ContextOpenGL* context);

	void update();

	void swapBuffers();

	void destroy();

	virtual bool enter();

	virtual void leave();

	virtual void deleteResource(IDeleteCallback* callback);

	virtual void deleteResources();

#if !defined(_WIN32) && !defined(__APPLE__)
	inline GLXContext getGLXContext() { return m_context; }
#elif defined(__APPLE__)
	inline AGLContext getAGLContext() { return m_context; }
#endif

private:
#if defined(_WIN32)
	HWND m_hWnd;
	HDC m_hDC;
	HGLRC m_hRC;
#elif defined(__APPLE__)
	WindowRef m_window;
	ControlRef m_control;
	AGLContext m_context;
#else	// LINUX
	Display* m_display;
	Window m_window;
	GLXContext m_context;
#endif

	static ThreadLocal ms_contextStack;
	std::vector< IDeleteCallback* > m_deleteResources;
};

	}
}

#endif	// traktor_render_ContextOpenGL_H
