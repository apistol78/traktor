#include <cstring>
#include "Core/Platform.h"
#include "Core/RefArray.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/TString.h"
#include "Core/Thread/Acquire.h"
#include "Render/OpenGL/ES2/ExtensionsGLES2.h"
#include "Render/OpenGL/ES2/Win32/ContextOpenGLES2.h"
#include "Render/OpenGL/ES2/Win32/Window.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const uint32_t c_maxMatchConfigs = 64;

typedef RefArray< ContextOpenGLES2 > context_stack_t;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ContextOpenGLES2", ContextOpenGLES2, Object)

ThreadLocal ContextOpenGLES2::ms_contextStack;

Ref< ContextOpenGLES2 > ContextOpenGLES2::createContext(const SystemApplication& sysapp, const RenderViewDefaultDesc& desc)
{
	Ref< ContextOpenGLES2 > context = new ContextOpenGLES2();

	context->m_window = new Window();
	if (!context->m_window->create())
		return 0;

	context->m_window->setTitle(!desc.title.empty() ? desc.title.c_str() : L"Traktor - OpenGL ES 2.0 Renderer");
	context->m_window->setWindowedStyle(desc.displayMode.width, desc.displayMode.height);

	context->m_display = eglGetDisplay(GetDC(*context->m_window));
	if (context->m_display == EGL_NO_DISPLAY)
	{
		context->m_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
		if (context->m_display == EGL_NO_DISPLAY)
		{
			EGLint error = eglGetError();
			log::error << L"Create OpenGL ES2.0 failed; unable to get EGL display (" << getEGLErrorString(error) << L")" << Endl;
			return 0;
		}
	}

	if (!eglInitialize(context->m_display, 0, 0)) 
	{
		EGLint error = eglGetError();
		log::error << L"Create OpenGL ES2.0 failed; unable to initialize EGL (" << getEGLErrorString(error) << L")" << Endl;
		return 0;
	}

	EGLConfig matchingConfigs[c_maxMatchConfigs];
	EGLint numMatchingConfigs = 0;

	if (desc.multiSample > 1)
	{
		const EGLint configAttribsWithMSAA[] =
		{
			EGL_LEVEL, 0,
			EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
			EGL_DEPTH_SIZE, desc.depthBits,
			EGL_STENCIL_SIZE, desc.stencilBits,
			EGL_SAMPLES, (EGLint)desc.multiSample,
			EGL_RED_SIZE, 8,
			EGL_GREEN_SIZE, 8,
			EGL_BLUE_SIZE, 8,
			EGL_ALPHA_SIZE, (desc.displayMode.colorBits >= 32) ? 8 : 0,
			EGL_NONE
		};

		EGLBoolean success = eglChooseConfig(
			context->m_display,
			configAttribsWithMSAA,
			matchingConfigs,
			c_maxMatchConfigs,
			&numMatchingConfigs
		);
		if (!success || numMatchingConfigs == 0)
			log::warning << L"No matching MSAA configurations found; MSAA disabled." << Endl;
	}

	if (numMatchingConfigs == 0)
	{
		const EGLint configAttribs[] =
		{
			EGL_LEVEL, 0,
			EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
			EGL_DEPTH_SIZE, desc.depthBits,
			EGL_STENCIL_SIZE, desc.stencilBits,
			EGL_RED_SIZE, 8,
			EGL_GREEN_SIZE, 8,
			EGL_BLUE_SIZE, 8,
			EGL_ALPHA_SIZE, (desc.displayMode.colorBits >= 32) ? 8 : 0,
			EGL_NONE
		};

		EGLBoolean success = eglChooseConfig(
			context->m_display,
			configAttribs,
			matchingConfigs,
			c_maxMatchConfigs,
			&numMatchingConfigs
		);
		if (!success)
		{
			EGLint error = eglGetError();
			log::error << L"Create OpenGL ES2.0 failed; unable to choose EGL config (" << getEGLErrorString(error) << L")." << Endl;
			return 0;
		}
	}

	if (numMatchingConfigs == 0)
	{
		EGLint error = eglGetError();
		log::error << L"Create OpenGL ES2.0 failed; no matching configurations." << Endl;
		return 0;
	}

	context->m_config = matchingConfigs[0];
	context->m_surface = eglCreateWindowSurface(context->m_display, context->m_config, *context->m_window, 0);
	if (context->m_surface == EGL_NO_SURFACE)
	{
		EGLint error = eglGetError();
		log::error << L"Create OpenGL ES2.0 context failed; unable to create EGL surface (" << getEGLErrorString(error) << L")" << Endl;
		return 0;
	}

	eglBindAPI(EGL_OPENGL_ES_API);

	const EGLint contextAttribs[] = 
	{
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};

	context->m_context = eglCreateContext(
		context->m_display,
		context->m_config,
		EGL_NO_CONTEXT,
		contextAttribs
	);
	if (context->m_context == EGL_NO_CONTEXT)
	{
		EGLint error = eglGetError();
		log::error << L"Create OpenGL ES2.0 context failed (1); unable to create EGL context (" << getEGLErrorString(error) << L")" << Endl;
		return 0;
	}

	eglQuerySurface(context->m_display, context->m_surface, EGL_WIDTH, &context->m_width);
	eglQuerySurface(context->m_display, context->m_surface, EGL_HEIGHT, &context->m_height);

	if (!context->enter())
		return 0;
	initializeExtensions();
	context->leave();

	log::info << L"OpenGL ES 2.0 render context created successfully" << Endl;
	return context;
}

Ref< ContextOpenGLES2 > ContextOpenGLES2::createContext(const SystemApplication& sysapp, const RenderViewEmbeddedDesc& desc)
{
	Ref< ContextOpenGLES2 > context = new ContextOpenGLES2();

	context->m_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (context->m_display == EGL_NO_DISPLAY)
	{
		EGLint error = eglGetError();
		log::error << L"Create OpenGL ES2.0 failed; unable to get EGL display (" << getEGLErrorString(error) << L")" << Endl;
		return 0;
	}

	if (!eglInitialize(context->m_display, 0, 0)) 
	{
		EGLint error = eglGetError();
		log::error << L"Create OpenGL ES2.0 failed; unable to initialize EGL (" << getEGLErrorString(error) << L")" << Endl;
		return 0;
	}

	EGLConfig matchingConfigs[c_maxMatchConfigs];
	EGLint numMatchingConfigs = 0;

	if (desc.multiSample > 1)
	{
		const EGLint configAttribsWithMSAA[] =
		{
			EGL_LEVEL, 0,
			EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
			EGL_BUFFER_SIZE, 32,
			EGL_DEPTH_SIZE, 16,
			EGL_STENCIL_SIZE, 4,
			EGL_SAMPLES, (EGLint)desc.multiSample,
			EGL_NONE
		};

		EGLBoolean success = eglChooseConfig(
			context->m_display,
			configAttribsWithMSAA,
			matchingConfigs,
			c_maxMatchConfigs,
			&numMatchingConfigs
		);
		if (!success || numMatchingConfigs == 0)
			log::warning << L"No matching MSAA configurations found; MSAA disabled." << Endl;
	}

	if (numMatchingConfigs == 0)
	{
		const EGLint configAttribs[] =
		{
			EGL_LEVEL, 0,
			EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
			EGL_BUFFER_SIZE, 32,
			EGL_DEPTH_SIZE, 16,
			EGL_STENCIL_SIZE, 4,
			EGL_NONE
		};

		EGLBoolean success = eglChooseConfig(
			context->m_display,
			configAttribs,
			matchingConfigs,
			c_maxMatchConfigs,
			&numMatchingConfigs
		);
		if (!success)
		{
			EGLint error = eglGetError();
			log::error << L"Create OpenGL ES2.0 failed; unable to create choose EGL config (" << getEGLErrorString(error) << L")." << Endl;
			return 0;
		}
	}

	if (numMatchingConfigs == 0)
	{
		EGLint error = eglGetError();
		log::error << L"Create OpenGL ES2.0 failed; no matching configurations." << Endl;
		return 0;
	}

	context->m_config = matchingConfigs[0];

	context->m_surface = eglCreateWindowSurface(context->m_display, context->m_config, (EGLNativeWindowType)desc.syswin.hWnd, 0);
	if (context->m_surface == EGL_NO_SURFACE)
	{
		EGLint error = eglGetError();
		log::error << L"Create OpenGL ES2.0 context failed; unable to create EGL surface (" << getEGLErrorString(error) << L")" << Endl;
		return 0;
	}

	eglQuerySurface(context->m_display, context->m_surface, EGL_WIDTH, &context->m_width);
	eglQuerySurface(context->m_display, context->m_surface, EGL_HEIGHT, &context->m_height);

	eglBindAPI(EGL_OPENGL_ES_API);

	const EGLint contextAttribs[] = 
	{
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};

	context->m_context = eglCreateContext(
		context->m_display,
		context->m_config,
		EGL_NO_CONTEXT,
		contextAttribs
	);
	if (context->m_context == EGL_NO_CONTEXT)
	{
		EGLint error = eglGetError();
		log::error << L"Create OpenGL ES2.0 context failed (2); unable to create EGL context (" << getEGLErrorString(error) << L")" << Endl;
		return 0;
	}

	if (!eglSurfaceAttrib(context->m_display, context->m_surface, EGL_SWAP_BEHAVIOR, EGL_BUFFER_DESTROYED))
		log::warning << L"Unable to specify swap behaviour on EGL surface; might affect performance." << Endl;

	if (!context->enter())
		return 0;
	initializeExtensions();
	context->leave();
    
	log::info << L"OpenGL ES 2.0 render context created successfully (embedded)" << Endl;
	return context;
}

bool ContextOpenGLES2::reset(int32_t width, int32_t height)
{
	m_width = width;
	m_height = height;

	if (m_primaryDepth != 0)
	{
		glDeleteRenderbuffers(1, &m_primaryDepth);
		m_primaryDepth = 0;
	}

	if (m_window)
	{
		eglDestroySurface(m_display, m_surface);
		m_surface = eglCreateWindowSurface(m_display, m_config, (EGLNativeWindowType)*m_window, 0);
	}

	return true;
}

bool ContextOpenGLES2::enter()
{
	if (!m_lock.wait())
		return false;

	context_stack_t* stack = static_cast< context_stack_t* >(ms_contextStack.get());
	if (!stack)
	{
		stack = new context_stack_t();
		ms_contextStack.set(stack);
	}

	if (!eglMakeCurrent(m_display, m_surface, m_surface, m_context))
	{
		EGLint error = eglGetError();
		log::error << L"Enter OpenGL ES2.0 context failed; " << getEGLErrorString(error) << Endl;
		m_lock.release();
		return false;
	}

	stack->push_back(this);
	return true;
}

void ContextOpenGLES2::leave()
{
	context_stack_t* stack = static_cast< context_stack_t* >(ms_contextStack.get());

	T_ASSERT (stack);
	T_ASSERT (!stack->empty());
	T_ASSERT (stack->back() == this);

	stack->pop_back();

	if (!stack->empty())
		eglMakeCurrent(m_display, stack->back()->m_surface, stack->back()->m_surface, stack->back()->m_context);
	else
		eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

	eglReleaseThread();
	m_lock.release();
}

void ContextOpenGLES2::deleteResource(IDeleteCallback* callback)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_deleteResources.push_back(callback);
}

void ContextOpenGLES2::deleteResources()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	if (!m_deleteResources.empty())
	{
		enter();
		for (std::vector< IDeleteCallback* >::iterator i = m_deleteResources.begin(); i != m_deleteResources.end(); ++i)
			(*i)->deleteResource();
		m_deleteResources.resize(0);
		leave();
	}
}

GLuint ContextOpenGLES2::createShaderObject(const char* shader, GLenum shaderType)
{
	Adler32 adler;
	adler.begin();
	adler.feed(shader, strlen(shader));
	adler.end();

	uint32_t hash = adler.get();

	std::map< uint32_t, GLuint >::const_iterator i = m_shaderObjects.find(hash);
	if (i != m_shaderObjects.end())
		return i->second;

	GLuint shaderObject = glCreateShader(shaderType);
	if (shaderObject == 0)
	{
		log::error << L"Failed to compile GLSL shader; glCreateShader returned 0" << Endl;
		return 0;
	}

	T_OGL_SAFE(glShaderSource(shaderObject, 1, &shader, NULL));
	T_OGL_SAFE(glCompileShader(shaderObject));

	char errorBuf[32000];
	GLint status;

	T_OGL_SAFE(glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &status));
	if (status == 0)
	{
		T_OGL_SAFE(glGetShaderInfoLog(shaderObject, sizeof(errorBuf), 0, errorBuf));
		log::error << L"Failed to compile GLSL shader:" << Endl;
		log::error << mbstows(errorBuf) << Endl;
		log::error << Endl;
		FormatMultipleLines(log::error, mbstows(shader));
		return 0;
	}

	m_shaderObjects.insert(std::make_pair(hash, shaderObject));
	return shaderObject;
}

int32_t ContextOpenGLES2::getWidth() const
{
	return m_width;
}

int32_t ContextOpenGLES2::getHeight() const
{
	return m_height;
}

void ContextOpenGLES2::swapBuffers()
{
	eglSwapBuffers(m_display, m_surface);
}

Semaphore& ContextOpenGLES2::lock()
{
	return m_lock;
}

void ContextOpenGLES2::bindPrimary()
{
	T_OGL_SAFE(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	T_OGL_SAFE(glViewport(
		0,
		0,
		getWidth(),
		getHeight()
	));

	if (!m_primaryDepth)
	{
		T_OGL_SAFE(glGenRenderbuffers(1, &m_primaryDepth));
		T_OGL_SAFE(glBindRenderbuffer(GL_RENDERBUFFER, m_primaryDepth));
		T_OGL_SAFE(glRenderbufferStorage(
			GL_RENDERBUFFER,
			GL_DEPTH_COMPONENT16,
			getWidth(),
			getHeight()
		));
	}
}

GLuint ContextOpenGLES2::getPrimaryDepth() const
{
	return m_primaryDepth;
}

ContextOpenGLES2::ContextOpenGLES2()
:	m_width(0)
,	m_height(0)
,	m_primaryDepth(0)
{
}

	}
}
