#ifndef traktor_render_ContextOpenGL_H
#define traktor_render_ContextOpenGL_H

#include <map>
#if defined(__LINUX__)
#	include <X11/Xlib.h>
#endif
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
	ContextOpenGL(ContextOpenGL* resourceContext, HWND hWnd, HDC hDC, HGLRC hRC);
#elif defined(__APPLE__)
	ContextOpenGL(ContextOpenGL* resourceContext, void* context);
#elif defined(__LINUX__)
	ContextOpenGL(ContextOpenGL* resourceContext, ::Display* display, ::Window window, GLXContext context);
#endif

	virtual ~ContextOpenGL();

#if !defined(__LINUX__)
	void update();
#else
	void update(int32_t width, int32_t height);
#endif

	void swapBuffers(bool waitVBlank);

	void destroy();

	GLuint createShaderObject(const char* shader, GLenum shaderType);

	uint32_t createRenderStateObject(const RenderStateOpenGL& renderState);

	uint32_t createSamplerStateObject(const SamplerStateOpenGL& samplerState);

	void bindRenderStateObject(uint32_t renderStateObject);

	void bindSamplerStateObject(GLenum textureTarget, uint32_t samplerStateObject, bool haveMips, GLfloat maxAnisotropy);

	void setPermitDepth(bool permitDepth);

	int32_t getPhysicalWidth() const;

	int32_t getPhysicalHeight() const;

	virtual bool enter();

	virtual void leave();

	virtual void deleteResource(IDeleteCallback* callback);

	virtual void deleteResources();

#if defined(_WIN32)
	HGLRC getGLRC() const { return m_hRC; }
#elif defined(__APPLE__)
	void* getGLContext() const { return m_context; }
#elif defined(__LINUX__)
	GLXContext getGLXContext() const { return m_context; }
#endif

private:
	ContextOpenGL* m_resourceContext;
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
	std::map< uint32_t, GLuint > m_shaderObjects;
	std::map< uint32_t, uint32_t > m_renderStateListCache;
	std::map< uint32_t, uint32_t > m_samplerStateListCache;
	std::vector< RenderStateOpenGL > m_renderStateList;
	std::vector< SamplerStateOpenGL > m_samplerStateList;
	std::vector< IDeleteCallback* > m_deleteResources;
	int32_t m_width;
	int32_t m_height;
	bool m_permitDepth;
	uint32_t m_currentRenderStateList;
};

	}
}

#endif	// traktor_render_ContextOpenGL_H
