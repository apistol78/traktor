#include "Core/RefArray.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/TString.h"
#include "Core/Thread/Acquire.h"
#include "Render/OpenGL/ES2/ContextOpenGLES2.h"

#if TARGET_OS_IPHONE
#	include "Render/OpenGL/ES2/IPhone/EAGLContextWrapper.h"
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

#if defined(T_OPENGL_ES2_HAVE_EGL)
#	if defined(_WIN32)
HWND ContextOpenGLES2::ms_hWnd = 0;
#	endif
EGLDisplay ContextOpenGLES2::ms_display = 0;
EGLConfig ContextOpenGLES2::ms_config = 0;
#endif

bool ContextOpenGLES2::initialize()
{
#if defined(T_OPENGL_ES2_HAVE_EGL)
#	if defined(_WIN32)
	ms_hWnd = CreateWindow(
		_T("RenderSystemOpenGLES2_FullScreen"),
		_T("Traktor 2.0 OpenGL ES 2.0 Renderer (Resource)"),
		WS_POPUPWINDOW,
		0,
		0,
		16,
		16,
		NULL,
		NULL,
		static_cast< HMODULE >(GetModuleHandle(NULL)),
		0
	);
	T_ASSERT (ms_hWnd != NULL);

	ms_display = eglGetDisplay(GetDC(ms_hWnd));
	if (ms_display == EGL_NO_DISPLAY) 
#	endif
	{
		ms_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
		if (ms_display == EGL_NO_DISPLAY)
		{
			EGLint error = eglGetError();
			log::error << L"Create OpenGL ES2.0 failed; unable to get EGL display (" << getEGLErrorString(error) << L")" << Endl;
			return false;
		}
	}

	if (!eglInitialize(ms_display, 0, 0)) 
	{
		EGLint error = eglGetError();
		log::error << L"Create OpenGL ES2.0 failed; unable to initialize EGL (" << getEGLErrorString(error) << L")" << Endl;
		return false;
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
		ms_display,
		configAttribs,
		matchingConfigs,
		c_maxMatchConfigs,
		&numMatchingConfigs
	);
	if (!success)
	{
		EGLint error = eglGetError();
		log::error << L"Create OpenGL ES2.0 failed; unable to create choose EGL config (" << getEGLErrorString(error) << L")" << Endl;
		return false;
	}

	if (numMatchingConfigs == 0)
	{
		EGLint error = eglGetError();
		log::error << L"Create OpenGL ES2.0 failed; no matching configurations" << Endl;
		return false;
	}

	ms_config = matchingConfigs[0];
#endif
	return true;
}

Ref< ContextOpenGLES2 > ContextOpenGLES2::createResourceContext()
{
#if defined(T_OPENGL_ES2_HAVE_EGL)
#	if defined(_WIN32)
	EGLSurface surface = eglCreateWindowSurface(ms_display, ms_config, ms_hWnd, 0);
#	else
	EGLint surfaceAttrs[] =
	{
		EGL_NONE
	};
	EGLSurface surface = eglCreatePbufferSurface(ms_display, ms_config, surfaceAttrs);
	if (surface == EGL_NO_SURFACE)
	{
		EGLint error = eglGetError();
		log::error << L"Create OpenGL ES2.0 context failed; unable to create EGL surface (" << getEGLErrorString(error) << L")" << Endl;
		return 0;
	}
#	endif

	eglBindAPI(EGL_OPENGL_ES_API);

	const EGLint contextAttribs[] = 
	{
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};

	EGLContext context = eglCreateContext(
		ms_display,
		ms_config,
		EGL_NO_CONTEXT,
		contextAttribs
	);
	if (context == EGL_NO_CONTEXT)
	{
		EGLint error = eglGetError();
		log::error << L"Create OpenGL ES2.0 context failed; unable to create EGL context (" << getEGLErrorString(error) << L")" << Endl;
		return 0;
	}

	return new ContextOpenGLES2(surface, context);

#elif TARGET_OS_IPHONE

	EAGLContextWrapper* wrapper = new EAGLContextWrapper();
	if (!wrapper->create())
		return 0;

	return new ContextOpenGLES2(wrapper);

#else

	return 0;

#endif
}

Ref< ContextOpenGLES2 > ContextOpenGLES2::createContext(
	ContextOpenGLES2* resourceContext,
	void* nativeWindowHandle
)
{
#if TARGET_OS_IPHONE

	EAGLContextWrapper* wrapper = new EAGLContextWrapper();
	if (!wrapper->create(
		resourceContext ? resourceContext->m_context : 0,
		nativeWindowHandle
	))
		return 0;

	return new ContextOpenGLES2(wrapper);

#elif defined(T_OPENGL_ES2_HAVE_EGL)

	EGLNativeWindowType nativeWindow = (EGLNativeWindowType)nativeWindowHandle;

	EGLSurface surface = eglCreateWindowSurface(ms_display, ms_config, nativeWindow, 0);
	if (surface == EGL_NO_SURFACE)
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

	EGLContext context = eglCreateContext(
		ms_display,
		ms_config,
		resourceContext ? resourceContext->m_context : EGL_NO_CONTEXT,
		contextAttribs
	);
	if (context == EGL_NO_CONTEXT)
	{
		EGLint error = eglGetError();
		log::error << L"Create OpenGL ES2.0 context failed; unable to create EGL context (" << getEGLErrorString(error) << L")" << Endl;
		return 0;
	}

	return new ContextOpenGLES2(surface, context);

#else
	return 0;
#endif
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

#if TARGET_OS_IPHONE
	if (!EAGLContextWrapper::setCurrent(m_context))
#elif defined(T_OPENGL_ES2_HAVE_EGL)
	if (!eglMakeCurrent(ms_display, m_surface, m_surface, m_context))
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

#if TARGET_OS_IPHONE

	if (!stack->empty())
		EAGLContextWrapper::setCurrent(stack->back()->m_context);
	else
		EAGLContextWrapper::setCurrent(0);

#elif defined(T_OPENGL_ES2_HAVE_EGL)

	eglMakeCurrent(ms_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglReleaseThread();

	if (!stack->empty())
		eglMakeCurrent(
			ms_display,
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
	return false;
}

int32_t ContextOpenGLES2::getWidth() const
{
#if TARGET_OS_IPHONE
	return m_context->getWidth();
#elif defined(T_OPENGL_ES2_HAVE_EGL)
	EGLint width;
	eglQuerySurface(ms_display, m_surface, EGL_WIDTH, &width);
	return width;
#else
	return 0;
#endif
}

int32_t ContextOpenGLES2::getHeight() const
{
#if TARGET_OS_IPHONE
	return m_context->getHeight();
#elif defined(T_OPENGL_ES2_HAVE_EGL)
	EGLint height;
	eglQuerySurface(ms_display, m_surface, EGL_HEIGHT, &height);
	return height;
#else
	return 0;
#endif
}

bool ContextOpenGLES2::getLandscape() const
{
#if TARGET_OS_IPHONE
	return m_context->getLandscape();
#else
	return false;
#endif
}

void ContextOpenGLES2::swapBuffers()
{
#if defined(T_OPENGL_ES2_HAVE_EGL)
	eglSwapBuffers(ms_display, m_surface);
#elif TARGET_OS_IPHONE
	m_context->swapBuffers();
#endif
}

Semaphore& ContextOpenGLES2::lock()
{
	return m_lock;
}

void ContextOpenGLES2::bindPrimary()
{
#if defined(TARGET_OS_IPHONE)
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
	return m_primaryDepth;
}

#if defined(TARGET_OS_IPHONE)
ContextOpenGLES2::ContextOpenGLES2(EAGLContextWrapper* context)
:	m_context(context)
{
}
#elif defined(T_OPENGL_ES2_HAVE_EGL)
ContextOpenGLES2::ContextOpenGLES2(EGLSurface surface, EGLContext context)
:	m_surface(surface)
,	m_context(context)
,	m_primaryDepth(0)
{
}
#endif

	}
}

#endif
