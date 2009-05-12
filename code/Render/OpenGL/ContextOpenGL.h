#ifndef traktor_render_ContextOpenGL_H
#define traktor_render_ContextOpenGL_H

#include <map>
#include "Core/Object.h"
#include "Render/OpenGL/Platform.h"

namespace traktor
{
	namespace render
	{

#define T_CONTEXT_SCOPE(context) ContextOpenGL::Scope __scope__(context);

/*! \brief OpenGL context.
 * \ingroup OGL
 */
class ContextOpenGL : public Object
{
public:
	/*! \brief Delete callback. 
	 * \ingroup OGL
	 *
	 * These are enqueued in the context
	 * and are invoked as soon as it's
	 * safe to actually delete resources.
	 */
	struct DeleteCallback
	{
		virtual void deleteResource() = 0;
	};

	/*! \brief Scoped enter/leave helper.
	 * \ingroup OGL
	 */
	struct Scope
	{
		ContextOpenGL* m_context;

		Scope(ContextOpenGL* context)
		:	m_context(context)
		{
			m_context->enter();
		}

		~Scope()
		{
			m_context->leave();
		}
	};

#if defined(_WIN32)
	ContextOpenGL(HWND hWnd, HDC hDC, HGLRC hRC);
#elif defined(__APPLE__)
	ContextOpenGL(WindowRef window, ControlRef control, AGLContext context);
#else	// LINUX
	ContextOpenGL(Display* display, Window window, GLXContext context);
#endif

	virtual ~ContextOpenGL();

	bool enter();

	void leave();

	void share(ContextOpenGL* context);

	void update();

	void swapBuffers();

	void destroy();

	void deleteResource(DeleteCallback* callback);

	void deleteResources();

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
	uint32_t m_count;
	uint32_t m_ownerThreadId;
	static std::map< uint32_t, ContextOpenGL* > ms_contextThreads;
#elif defined(__APPLE__)
	WindowRef m_window;
	ControlRef m_control;
	AGLContext m_context;
#else	// LINUX
	Display* m_display;
	Window m_window;
	GLXContext m_context;
#endif

	std::vector< DeleteCallback* > m_deleteResources;
};

	}
}

#endif	// traktor_render_ContextOpenGL_H
