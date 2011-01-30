#ifndef traktor_render_ContextOpenGL_H
#define traktor_render_ContextOpenGL_H

#include <map>
#include "Core/Object.h"
#include "Core/Thread/Semaphore.h"
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
	T_RTTI_CLASS;

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

	void swapBuffers(bool waitVBlank);

	void destroy();
	
	void enable(GLenum state);
	
	void disable(GLenum state);
	
	void setRenderState(const RenderState& renderState, bool invertCull);
	
	GLhandleARB createShaderObject(const char* shader, GLenum shaderType);

	int32_t getWidth() const;

	int32_t getHeight() const;

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
	Semaphore m_lock;
	std::map< GLenum, bool > m_enableStates;
	RenderState m_renderState;
	std::map< uint32_t, GLhandleARB > m_shaderObjects;
	GLuint m_currentStateList;
	std::vector< IDeleteCallback* > m_deleteResources;
	int32_t m_width;
	int32_t m_height;
};

	}
}

#endif	// traktor_render_ContextOpenGL_H
