#include <cstring>
#include "Core/RefArray.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/TString.h"
#include "Core/Thread/Acquire.h"
#include "Render/OpenGL/ES2/ContextOpenGLES2.h"

#if defined(_WIN32)
#	include "Render/OpenGL/ES2/Win32/Window.h"
#elif defined(__IOS__)
#	include "Render/OpenGL/ES2/iOS/EAGLContextWrapper.h"
#elif defined(__PNACL__)
#	include "Render/OpenGL/ES2/PNaCl/PPContextWrapper.h"
#endif

#if !defined(T_OFFLINE_ONLY)

namespace traktor
{
	namespace render
	{
		namespace
		{

const uint32_t c_maxMatchConfigs = 64;

typedef RefArray< ContextOpenGLES2 > context_stack_t;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ContextOpenGLES2", ContextOpenGLES2, IContext)

ThreadLocal ContextOpenGLES2::ms_contextStack;

Ref< ContextOpenGLES2 > ContextOpenGLES2::createResourceContext(void* nativeHandle)
{
	Ref< ContextOpenGLES2 > context = new ContextOpenGLES2();

#if defined(T_OPENGL_ES2_HAVE_EGL)

#	if defined(_WIN32)

	context->m_window = new Window();
	if (!context->m_window->create())
		return 0;

	context->m_display = eglGetDisplay(GetDC(*context->m_window));
	if (context->m_display == EGL_NO_DISPLAY)

#	endif

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

	const EGLint configAttribs[] =
	{
		EGL_LEVEL, 0,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_NATIVE_RENDERABLE, EGL_FALSE,
		EGL_DEPTH_SIZE, 24,
		EGL_STENCIL_SIZE, 4,
		EGL_NONE
	};

	EGLConfig matchingConfigs[c_maxMatchConfigs];
	EGLint numMatchingConfigs = 0;

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
		log::error << L"Create OpenGL ES2.0 failed; unable to create choose EGL config (" << getEGLErrorString(error) << L")" << Endl;
		return 0;
	}

	if (numMatchingConfigs == 0)
	{
		EGLint error = eglGetError();
		log::error << L"Create OpenGL ES2.0 failed; no matching configurations" << Endl;
		return 0;
	}

	context->m_config = matchingConfigs[0];

#	if defined(_WIN32)
	context->m_surface = eglCreateWindowSurface(context->m_display, context->m_config, *context->m_window, 0);
#	elif defined(__EMSCRIPTEN__)
	context->m_surface = eglCreateWindowSurface(context->m_display, context->m_config, 0, 0);
#	else
	EGLint surfaceAttrs[] =
	{
		EGL_NONE
	};
	context->m_surface = eglCreatePbufferSurface(context->m_display, context->m_config, surfaceAttrs);
#	endif

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
		log::error << L"Create OpenGL ES2.0 context failed; unable to create EGL context (" << getEGLErrorString(error) << L")" << Endl;
		return 0;
	}

#elif defined(__IOS__)

	context->m_context = new EAGLContextWrapper();
	if (!context->m_context->create())
		return 0;

#elif defined(__PNACL__)

	context->m_context = PPContextWrapper::createResourceContext((pp::Instance*)nativeHandle);
	if (!context->m_context)
		return 0;

#endif

	const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
    if (extensions)
        log::info << L"GL_EXTENSIONS = " << mbstows(extensions) << Endl;

	return context;
}

Ref< ContextOpenGLES2 > ContextOpenGLES2::createContext(ContextOpenGLES2* resourceContext, void* nativeHandle, const RenderViewDefaultDesc& desc)
{
	Ref< ContextOpenGLES2 > context = new ContextOpenGLES2();

#if defined(T_OPENGL_ES2_HAVE_EGL)

#	if !defined(__ANDROID__) && !defined(__EMSCRIPTEN__)
	context->m_window = resourceContext->m_window;
#	endif
	context->m_display = resourceContext->m_display;
	context->m_config = resourceContext->m_config;
	context->m_surface = resourceContext->m_surface;

#	if !defined(__ANDROID__) && !defined(__EMSCRIPTEN__)
	context->m_window->setTitle(desc.title.c_str());
	context->m_window->setWindowedStyle(desc.displayMode.width, desc.displayMode.height);
#	endif

	eglBindAPI(EGL_OPENGL_ES_API);

	const EGLint contextAttribs[] = 
	{
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};

	context->m_context = eglCreateContext(
		context->m_display,
		context->m_config,
		resourceContext->m_context,
		contextAttribs
	);
	if (context->m_context == EGL_NO_CONTEXT)
	{
		EGLint error = eglGetError();
		log::error << L"Create OpenGL ES2.0 context failed; unable to create EGL context (" << getEGLErrorString(error) << L")" << Endl;
		return 0;
	}

#	if defined(_WIN32)
	eglQuerySurface(context->m_display, context->m_surface, EGL_WIDTH, &context->m_width);
	eglQuerySurface(context->m_display, context->m_surface, EGL_HEIGHT, &context->m_height);
#	endif

#elif defined(__PNACL__)

	context->m_context = PPContextWrapper::createRenderContext(
		(pp::Instance*)nativeHandle,
		resourceContext->m_context
	);
	if (!context->m_context)
		return 0;

#else

	return 0;

#endif

	return context;
}

Ref< ContextOpenGLES2 > ContextOpenGLES2::createContext(ContextOpenGLES2* resourceContext, void* nativeHandle, const RenderViewEmbeddedDesc& desc)
{
	Ref< ContextOpenGLES2 > context = new ContextOpenGLES2();

#if defined(T_OPENGL_ES2_HAVE_EGL)

	context->m_display = resourceContext->m_display;
	context->m_config = resourceContext->m_config;

	context->m_surface = eglCreateWindowSurface(resourceContext->m_display, resourceContext->m_config, (EGLNativeWindowType)desc.nativeWindowHandle, 0);
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
		resourceContext->m_display,
		resourceContext->m_config,
		resourceContext->m_context,
		contextAttribs
	);
	if (context->m_context == EGL_NO_CONTEXT)
	{
		EGLint error = eglGetError();
		log::error << L"Create OpenGL ES2.0 context failed; unable to create EGL context (" << getEGLErrorString(error) << L")" << Endl;
		return 0;
	}

#elif defined(__IOS__)

	context->m_context = new EAGLContextWrapper();
	if (!context->m_context->create(
		resourceContext->m_context,
		desc.nativeWindowHandle
	))
		return 0;

#elif defined(__PNACL__)

	context->m_context = PPContextWrapper::createRenderContext(
		(pp::Instance*)nativeHandle,
		resourceContext->m_context
	);
	if (!context->m_context)
		return 0;

#endif
    
    const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
    if (extensions)
        log::info << L"GL_EXTENSIONS = " << mbstows(extensions) << Endl;

	return context;
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

#if defined(__IOS__)
	if (!EAGLContextWrapper::setCurrent(m_context))
#elif defined(__PNACL__)
	if (!m_context->makeCurrent())
#elif defined(T_OPENGL_ES2_HAVE_EGL)
	if (!eglMakeCurrent(m_display, m_surface, m_surface, m_context))
#endif
	{
#if defined(T_OPENGL_ES2_HAVE_EGL)
		EGLint error = eglGetError();
		log::error << L"Enter OpenGL ES2.0 context failed; " << getEGLErrorString(error) << Endl;
#endif
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

#if defined(__IOS__)
	if (!stack->empty())
		EAGLContextWrapper::setCurrent(stack->back()->m_context);
	else
		EAGLContextWrapper::setCurrent(0);
#elif defined(__PNACL__)
	if (!stack->empty())
		stack->back()->m_context->makeCurrent();
#elif defined(T_OPENGL_ES2_HAVE_EGL)
	eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
#	if !defined(__EMSCRIPTEN__)
	eglReleaseThread();
#	endif
	if (!stack->empty())
		eglMakeCurrent(
			m_display,
			stack->back()->m_surface,
			stack->back()->m_surface,
			stack->back()->m_context
		);
#endif

	m_lock.release();
}

void ContextOpenGLES2::deleteResource(IDeleteCallback* callback)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
#if !defined(__IOS__)
	m_deleteResources.push_back(callback);
#else
	enter();
	callback->deleteResource();
	leave();
#endif
}

void ContextOpenGLES2::deleteResources()
{
#if !defined(__IOS__)
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	if (!m_deleteResources.empty())
	{
		enter();
		for (std::vector< IDeleteCallback* >::iterator i = m_deleteResources.begin(); i != m_deleteResources.end(); ++i)
			(*i)->deleteResource();
		m_deleteResources.resize(0);
		leave();
	}
#endif
}

GLuint ContextOpenGLES2::createShaderObject(const char* shader, GLenum shaderType)
{
	char errorBuf[32000];
	GLint status;

	Adler32 adler;
	adler.begin();
	adler.feed(shader, strlen(shader));
	adler.end();

	uint32_t hash = adler.get();

	std::map< uint32_t, GLuint >::const_iterator i = m_shaderObjects.find(hash);
	if (i != m_shaderObjects.end())
		return i->second;

	GLuint shaderObject = glCreateShader(shaderType);

	T_OGL_SAFE(glShaderSource(shaderObject, 1, &shader, NULL));
	T_OGL_SAFE(glCompileShader(shaderObject));
	T_OGL_SAFE(glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &status));
	if (status == 0)
	{
		T_OGL_SAFE(glGetShaderInfoLog(shaderObject, sizeof(errorBuf), 0, errorBuf));
		log::error << L"GLSL fragment shader compile failed :" << Endl;
		log::error << mbstows(errorBuf) << Endl;
		log::error << Endl;
		FormatMultipleLines(log::error, mbstows(shader));
		return false;
	}

	m_shaderObjects.insert(std::make_pair(hash, shaderObject));
	return shaderObject;
}

bool ContextOpenGLES2::resize(int32_t width, int32_t height)
{
#if defined(__PNACL__)
	return m_context->resize(width, height);
#elif defined(_WIN32)
	m_width = width;
	m_height = height;
	m_primaryDepth = 0;	// \fixme Should release depth buffer!
	return true;
#else
	return false;
#endif
}

int32_t ContextOpenGLES2::getWidth() const
{
#if defined(__IOS__) || defined(__PNACL__)
	return m_context->getWidth();
#elif defined(_WIN32)
	return m_width;
#elif defined(T_OPENGL_ES2_HAVE_EGL)
	EGLint width;
	eglQuerySurface(m_display, m_surface, EGL_WIDTH, &width);
	return width;
#else
	return 0;
#endif
}

int32_t ContextOpenGLES2::getHeight() const
{
#if defined(__IOS__) || defined(__PNACL__)
	return m_context->getHeight();
#elif defined(_WIN32)
	return m_height;
#elif defined(T_OPENGL_ES2_HAVE_EGL)
	EGLint height;
	eglQuerySurface(m_display, m_surface, EGL_HEIGHT, &height);
	return height;
#else
	return 0;
#endif
}

bool ContextOpenGLES2::getLandscape() const
{
#if defined(__IOS__)
	return m_context->getLandscape();
#else
	return false;
#endif
}

void ContextOpenGLES2::swapBuffers()
{
#if defined(T_OPENGL_ES2_HAVE_EGL)
	eglSwapBuffers(m_display, m_surface);
#elif defined(__IOS__) || defined(__PNACL__)
	m_context->swapBuffers();
#endif
}

Semaphore& ContextOpenGLES2::lock()
{
	return m_lock;
}

void ContextOpenGLES2::bindPrimary()
{
#if defined(__IOS__) || defined(__PNACL__)
	T_OGL_SAFE(glBindFramebuffer(GL_FRAMEBUFFER, m_context->getFrameBuffer()));
#else
	T_OGL_SAFE(glBindFramebuffer(GL_FRAMEBUFFER, 0));
#endif

	T_OGL_SAFE(glViewport(
		0,
		0,
		getWidth(),
		getHeight()
	));

#if defined(T_OPENGL_ES2_HAVE_EGL)
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
#endif
}

GLuint ContextOpenGLES2::getPrimaryDepth() const
{
#if defined(T_OPENGL_ES2_HAVE_EGL)
	return m_primaryDepth;
#elif defined(__IOS__) || defined(__PNACL__)
	return m_context->getDepthBuffer();
#else
	return 0;
#endif
}

#if defined(__IOS__)
ContextOpenGLES2::ContextOpenGLES2()
:	m_context(0)
{
}
#elif defined(__PNACL__)
ContextOpenGLES2::ContextOpenGLES2()
{
}
#elif defined(T_OPENGL_ES2_HAVE_EGL)
ContextOpenGLES2::ContextOpenGLES2()
#	if defined(_WIN32)
:	m_width(0)
,	m_height(0)
,	m_primaryDepth(0)
#	else
:	m_primaryDepth(0)
#	endif
{
}
#endif

	}
}

#endif
