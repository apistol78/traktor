#ifndef traktor_render_ContextOpenGL_H
#define traktor_render_ContextOpenGL_H

#include <map>
#include "Core/Object.h"
#include "Core/Thread/ThreadLocal.h"
#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/IContext.h"
#include "Render/OpenGL/TypesOpenGL.h"

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
	ContextOpenGL(void* context);
#else	// LINUX
	ContextOpenGL(Display* display, Window window, GLXContext context);
#endif

	virtual ~ContextOpenGL();

	void share(ContextOpenGL* context);

	void update();

	void swapBuffers();

	void destroy();
	
	void enable(GLenum state);
	
	void disable(GLenum state);
	
	GLuint createStateList(const RenderState& renderState);
	
	void callStateList(GLuint stateList);

	virtual bool enter();

	virtual void leave();

	virtual void deleteResource(IDeleteCallback* callback);

	virtual void deleteResources();

#if !defined(_WIN32) && !defined(__APPLE__)
	inline GLXContext getGLXContext() { return m_context; }
#elif defined(__APPLE__)
	inline void* getGLContext() { return m_context; }
#endif

private:
#if defined(_WIN32)
	HWND m_hWnd;
	HDC m_hDC;
	HGLRC m_hRC;
#elif defined(__APPLE__)
	void* m_context;
#else	// LINUX
	Display* m_display;
	Window m_window;
	GLXContext m_context;
#endif

	static ThreadLocal ms_contextStack;
	std::map< GLenum, bool > m_enableStates;
	std::map< uint32_t, GLuint > m_stateLists;
	GLuint m_currentStateList;
	std::vector< IDeleteCallback* > m_deleteResources;
};

	}
}

#endif	// traktor_render_ContextOpenGL_H
