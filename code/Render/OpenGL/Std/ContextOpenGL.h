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
#elif defined(__LINUX__)
	ContextOpenGL(Display* display, GLXDrawable drawable, GLXContext context);
#endif

	virtual ~ContextOpenGL();

	void share(ContextOpenGL* context);

	void update();

	void swapBuffers(bool waitVBlank);

	void destroy();

	GLhandleARB createShaderObject(const char* shader, GLenum shaderType);

	GLuint createStateList(const RenderState& renderState);

	void callStateList(GLuint listBase);

	void setPermitDepth(bool permitDepth);

	int32_t getWidth() const;

	int32_t getHeight() const;

	virtual bool enter();

	virtual void leave();

	virtual void deleteResource(IDeleteCallback* callback);

	virtual void deleteResources();

#if defined(__APPLE__)
	inline void* getGLContext() { return m_context; }
#elif defined(__LINUX__)
	inline GLXContext getGLXContext() { return m_context; }
#endif

private:
#if defined(_WIN32)
	HWND m_hWnd;
	HDC m_hDC;
	HGLRC m_hRC;
#elif defined(__APPLE__)
	void* m_context;
#elif defined(__LINUX__)
	Display* m_display;
	GLXDrawable m_drawable;
	GLXContext m_context;
#endif

	static ThreadLocal ms_contextStack;
	Semaphore m_lock;
	std::map< uint32_t, GLhandleARB > m_shaderObjects;
	std::map< uint32_t, GLuint > m_stateLists;
	std::vector< IDeleteCallback* > m_deleteResources;
	int32_t m_width;
	int32_t m_height;
	bool m_permitDepth;
	GLuint m_currentList;
};

	}
}

#endif	// traktor_render_ContextOpenGL_H
