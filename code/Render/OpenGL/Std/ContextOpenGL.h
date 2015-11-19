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
#include "Render/OpenGL/TypesOpenGL.h"

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

	/*! \brief Delete callback. 
	 * \ingroup OGL
	 *
	 * These are enqueued in the context
	 * and are invoked as soon as it's
	 * safe to actually delete resources.
	 */
	struct IDeleteCallback
	{
		virtual void deleteResource() = 0;
	};

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

	void bindSamplerStateObject(GLenum textureTarget, uint32_t samplerStateObject, uint32_t stage, bool haveMips);

	void setMaxAnisotropy(GLfloat maxAnisotropy);

	void setPermitDepth(bool permitDepth);

	int32_t getPhysicalWidth() const;

	int32_t getPhysicalHeight() const;

	bool enter();

	void leave();

	void deleteResource(IDeleteCallback* callback);

	void deleteResources();

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

	struct SamplerStateObject
	{
		union
		{
			struct
			{
				GLuint withMips;
				GLuint noMips;
			};
			GLuint samplers[2];
		};
	};

	static ThreadLocal ms_contextStack;
	Semaphore m_lock;
	std::map< uint32_t, GLuint > m_shaderObjects;
	std::map< uint32_t, uint32_t > m_renderStateListCache;
	//std::map< uint32_t, uint32_t > m_samplerStateListCache;
	std::map< uint32_t, SamplerStateObject > m_samplerStateObjects;
	std::vector< RenderStateOpenGL > m_renderStateList;
	//std::vector< SamplerStateOpenGL > m_samplerStateList;
	std::vector< IDeleteCallback* > m_deleteResources;
	int32_t m_width;
	int32_t m_height;
	GLfloat m_maxAnisotropy;
	bool m_permitDepth;
	uint32_t m_currentRenderStateList;
};

	}
}

#endif	// traktor_render_ContextOpenGL_H
