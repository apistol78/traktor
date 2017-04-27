/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ContextOpenGLES2_H
#define traktor_render_ContextOpenGLES2_H

#include <map>
#include "Core/Object.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/ThreadLocal.h"
#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/TypesOpenGL.h"

namespace traktor
{
	namespace render
	{

/*! \brief Android OpenGL ES2 context.
 * \ingroup OGL
 */
class ContextOpenGLES2 : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Scoped enter/leave helper.
	 * \ingroup OGL
	 */
	struct Scope
	{
		ContextOpenGLES2* m_context;

		Scope(ContextOpenGLES2* context)
		:	m_context(context)
		{
			bool result = m_context->enter();
			T_FATAL_ASSERT_M (result, L"Unable to set OpenGL ES2 context!");
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

	static Ref< ContextOpenGLES2 > createContext(const SystemApplication& sysapp, const RenderViewDefaultDesc& desc);

	static Ref< ContextOpenGLES2 > createContext(const SystemApplication& sysapp, const RenderViewEmbeddedDesc& desc);

	bool reset(int32_t width, int32_t height);

	bool enter();

	void leave();

	void deleteResource(IDeleteCallback* callback);

	void deleteResources();

	GLuint createShaderObject(const char* shader, GLenum shaderType);

	int32_t getWidth() const;

	int32_t getHeight() const;

	void swapBuffers();

	Semaphore& lock();
	
	void bindPrimary();

	GLuint getPrimaryDepth() const;

private:
	static ThreadLocal ms_contextStack;
	SystemWindow m_syswin;
	EGLDisplay m_display;
	EGLConfig m_config;
	EGLSurface m_surface;
	EGLContext m_context;
	GLenum m_primaryDepthFormat;
	GLuint m_primaryDepth;
	Semaphore m_lock;
	std::vector< IDeleteCallback* > m_deleteResources;
	std::map< uint32_t, GLuint > m_shaderObjects;

	ContextOpenGLES2();
};

	}
}

#endif	// traktor_render_ContextOpenGLES2_H
