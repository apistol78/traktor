#include "Core/RefArray.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/TString.h"
#include "Core/Thread/Acquire.h"
#include "Render/OpenGL/ES2/ContextOpenGLES2.h"

#if TARGET_OS_IPHONE
#	include "Render/OpenGL/ES2/IPhone/EAGLContextWrapper.h"
#endif

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

Ref< ContextOpenGLES2 > ContextOpenGLES2::createResourceContext()
{
#if defined(T_OPENGL_ES2_HAVE_EGL)
	EGLDisplay display;

#	if defined(_WIN32)
	HWND nativeWindow = CreateWindow(
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
	T_ASSERT (nativeWindow != NULL);

	display = eglGetDisplay(GetDC(nativeWindow));
	if (display == EGL_NO_DISPLAY) 
#	endif
	{
		display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
		if (display == EGL_NO_DISPLAY)
		{
			EGLint error = eglGetError();
			log::error << L"Create OpenGL ES2.0 context failed; unable to get EGL display (" << getEGLErrorString(error) << L")" << Endl;
			return 0;
		}
	}

	if (!eglInitialize(display, 0, 0)) 
	{
		EGLint error = eglGetError();
		log::error << L"Create OpenGL ES2.0 context failed; unable to initialize EGL (" << getEGLErrorString(error) << L")" << Endl;
		return 0;
	}

	const EGLint configAttribs[] =
	{
		EGL_LEVEL, 0,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_NATIVE_RENDERABLE, EGL_FALSE,
		EGL_DEPTH_SIZE, EGL_DONT_CARE,
		EGL_NONE
	};

	EGLConfig matchingConfigs[c_maxMatchConfigs];
	EGLint numMatchingConfigs = 0;

	EGLBoolean success = eglChooseConfig(
		display,
		configAttribs,
		matchingConfigs,
		c_maxMatchConfigs,
		&numMatchingConfigs
	);
	if (!success)
	{
		EGLint error = eglGetError();
		log::error << L"Create OpenGL ES2.0 context failed; unable to create choose EGL config (" << getEGLErrorString(error) << L")" << Endl;
		return 0;
	}

	if (numMatchingConfigs == 0)
	{
		EGLint error = eglGetError();
		log::error << L"Create OpenGL ES2.0 context failed; no matching configurations" << Endl;
		return 0;
	}

	EGLConfig config = matchingConfigs[0];

#	if defined(_WIN32)
	EGLSurface surface = eglCreateWindowSurface(display, config, nativeWindow, 0);
#	else
	EGLint surfaceAttrs[] =
	{
		EGL_NONE
	};
	EGLSurface surface = eglCreatePbufferSurface(display, config, surfaceAttrs);
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
		display,
		config,
		EGL_NO_CONTEXT,
		contextAttribs
	);
	if (context == EGL_NO_CONTEXT)
	{
		EGLint error = eglGetError();
		log::error << L"Create OpenGL ES2.0 context failed; unable to create EGL context (" << getEGLErrorString(error) << L")" << Endl;
		return 0;
	}

	return new ContextOpenGLES2(display, surface, context);

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
	void* nativeWindowHandle,
	uint32_t depthBits,
	uint32_t stencilBits
)
{
#if TARGET_OS_IPHONE

	EAGLContextWrapper* wrapper = new EAGLContextWrapper();
	if (!wrapper->create(
		resourceContext ? resourceContext->m_context : 0,
		nativeWindowHandle,
		depthBits != 0
	))
		return 0;

	return new ContextOpenGLES2(wrapper);

#elif defined(T_OPENGL_ES2_HAVE_EGL)

	EGLNativeWindowType nativeWindow = (EGLNativeWindowType)nativeWindowHandle;
	EGLDisplay display;

#	if defined(_WIN32)
	display = eglGetDisplay(GetDC(nativeWindow));
	if (display == EGL_NO_DISPLAY) 
#	endif
	{
		display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
		if (display == EGL_NO_DISPLAY) 
		{
			EGLint error = eglGetError();
			log::error << L"Create OpenGL ES2.0 context failed; unable to get EGL display (" << getEGLErrorString(error) << L")" << Endl;
			return 0;
		}
	}

	if (!eglInitialize(display, 0, 0)) 
	{
		EGLint error = eglGetError();
		log::error << L"Create OpenGL ES2.0 context failed; unable to initialize EGL (" << getEGLErrorString(error) << L")" << Endl;
		return 0;
	}

	const EGLint configAttribs[] =
	{
		EGL_LEVEL, 0,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_NATIVE_RENDERABLE, EGL_FALSE,
		EGL_DEPTH_SIZE, depthBits,
		EGL_STENCIL_SIZE, stencilBits,
		EGL_NONE
	};

	EGLConfig matchingConfigs[c_maxMatchConfigs];
	EGLint numMatchingConfigs = 0;

	EGLBoolean success = eglChooseConfig(
		display,
		configAttribs,
		matchingConfigs,
		c_maxMatchConfigs,
		&numMatchingConfigs
	);
	if (!success)
	{
		EGLint error = eglGetError();
		log::error << L"Create OpenGL ES2.0 context failed; unable to create choose EGL config (" << getEGLErrorString(error) << L")" << Endl;
		return 0;
	}

	if (numMatchingConfigs == 0)
	{
		EGLint error = eglGetError();
		log::error << L"Create OpenGL ES2.0 context failed; no matching configurations" << Endl;
		return 0;
	}

	EGLConfig config = matchingConfigs[0];

	EGLSurface surface = eglCreateWindowSurface(display, config, nativeWindow, 0);
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
		display,
		config,
		resourceContext ? resourceContext->m_context : EGL_NO_CONTEXT,
		contextAttribs
	);
	if (context == EGL_NO_CONTEXT)
	{
		EGLint error = eglGetError();
		log::error << L"Create OpenGL ES2.0 context failed; unable to create EGL context (" << getEGLErrorString(error) << L")" << Endl;
		return 0;
	}

	return new ContextOpenGLES2(display, surface, context);

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

#if TARGET_OS_IPHONE

	if (!stack->empty())
		EAGLContextWrapper::setCurrent(stack->back()->m_context);
	else
		EAGLContextWrapper::setCurrent(0);

#elif defined(T_OPENGL_ES2_HAVE_EGL)

	eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglReleaseThread();

	if (!stack->empty())
		eglMakeCurrent(
			stack->back()->m_display,
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
	if (!m_context->landscape())
		return m_context->getWidth();
	else
		return m_context->getHeight();
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
#if TARGET_OS_IPHONE
	if (!m_context->landscape())
		return m_context->getHeight();
	else
		return m_context->getWidth();
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
#if TARGET_OS_IPHONE
	return m_context->landscape();
#else
	return false;
#endif
}

void ContextOpenGLES2::swapBuffers()
{
#if defined(T_OPENGL_ES2_HAVE_EGL)
	eglSwapBuffers(m_display, m_surface);
#elif TARGET_OS_IPHONE
	m_context->swapBuffers();
#endif
}

Semaphore& ContextOpenGLES2::lock()
{
	return m_lock;
}

void ContextOpenGLES2::setRenderState(const RenderState& renderState)
{
	if (renderState.cullFaceEnable)
	{
		if (!m_renderState.cullFaceEnable)
		{
			T_OGL_SAFE(glEnable(GL_CULL_FACE));
			m_renderState.cullFaceEnable = true;
		}
		if (renderState.cullFace != m_renderState.cullFace)
		{
			T_OGL_SAFE(glCullFace(renderState.cullFace));
			m_renderState.cullFace = renderState.cullFace;
		}
	}
	else
	{
		if (m_renderState.cullFaceEnable)
		{
			T_OGL_SAFE(glDisable(GL_CULL_FACE));
			m_renderState.cullFaceEnable = false;
		}
	}

	if (renderState.blendEnable)
	{
		if (!m_renderState.blendEnable)
		{
			T_OGL_SAFE(glEnable(GL_BLEND));
			m_renderState.blendEnable = true;
		}
		if (renderState.blendFuncSrc != m_renderState.blendFuncSrc || renderState.blendFuncDest != m_renderState.blendFuncDest)
		{
			T_OGL_SAFE(glBlendFunc(renderState.blendFuncSrc, renderState.blendFuncDest));
			m_renderState.blendFuncSrc = renderState.blendFuncSrc;
			m_renderState.blendFuncDest = renderState.blendFuncDest;
		}
		if (renderState.blendEquation != m_renderState.blendEquation)
		{
			T_OGL_SAFE(glBlendEquation(renderState.blendEquation));
			m_renderState.blendEquation = renderState.blendEquation;
		}
	}
	else
	{
		if (m_renderState.blendEnable)
		{
			T_OGL_SAFE(glDisable(GL_BLEND));
			m_renderState.blendEnable = false;
		}
	}

	if (renderState.depthTestEnable)
	{
		if (!m_renderState.depthTestEnable)
		{
			T_OGL_SAFE(glEnable(GL_DEPTH_TEST));
			m_renderState.depthTestEnable = true;
		}
		if (renderState.depthFunc != m_renderState.depthFunc)
		{
			T_OGL_SAFE(glDepthFunc(renderState.depthFunc));
			m_renderState.depthFunc = renderState.depthFunc;
		}
	}
	else
	{
		if (m_renderState.depthTestEnable)
		{
			T_OGL_SAFE(glDisable(GL_DEPTH_TEST));
			m_renderState.depthTestEnable = false;
		}
	}

	if (renderState.colorMask != m_renderState.colorMask)
	{
		T_OGL_SAFE(glColorMask(
			(renderState.colorMask & RenderState::CmRed) ? GL_TRUE : GL_FALSE,
			(renderState.colorMask & RenderState::CmGreen) ? GL_TRUE : GL_FALSE,
			(renderState.colorMask & RenderState::CmBlue) ? GL_TRUE : GL_FALSE,
			(renderState.colorMask & RenderState::CmAlpha) ? GL_TRUE : GL_FALSE
		));
		m_renderState.colorMask = renderState.colorMask;
	}

	if (renderState.depthMask != m_renderState.depthMask)
	{
		T_OGL_SAFE(glDepthMask(renderState.depthMask));
		m_renderState.depthMask = renderState.depthMask;
	}

	/*
	if (renderState.stencilTestEnable)
	{
		if (!m_renderState.stencilTestEnable)
		{
			T_OGL_SAFE(glEnable(GL_STENCIL_TEST));
			m_renderState.stencilTestEnable = true;
		}
		if (renderState.stencilFunc != m_renderState.stencilFunc || renderState.stencilRef != m_renderState.stencilRef)
		{
			T_OGL_SAFE(glStencilFunc(renderState.stencilFunc, renderState.stencilRef, ~0UL));
			m_renderState.stencilFunc = renderState.stencilFunc;
			m_renderState.stencilRef = renderState.stencilRef;
		}
		if (renderState.stencilOpFail != m_renderState.stencilOpFail || renderState.stencilOpZFail != m_renderState.stencilOpZFail || renderState.stencilOpZPass != m_renderState.stencilOpZPass)
		{
			T_OGL_SAFE(glStencilOp(
				renderState.stencilOpFail,
				renderState.stencilOpZFail,
				renderState.stencilOpZPass
			));
			m_renderState.stencilOpFail = renderState.stencilOpFail;
			m_renderState.stencilOpZFail = renderState.stencilOpZFail;
			m_renderState.stencilOpZPass = renderState.stencilOpZPass;
		}
	}
	else
	{
		if (m_renderState.stencilTestEnable)
		{
			T_OGL_SAFE(glDisable(GL_STENCIL_TEST));
			m_renderState.stencilTestEnable = false;
		}
	}
	*/
}

void ContextOpenGLES2::bindPrimary()
{
#if defined(TARGET_OS_IPHONE)
	T_OGL_SAFE(glBindFramebuffer(GL_FRAMEBUFFER, m_context->getFrameBuffer()));
#else
	T_OGL_SAFE(glBindFramebuffer(GL_FRAMEBUFFER, 0));
#endif
}

#if defined(TARGET_OS_IPHONE)
ContextOpenGLES2::ContextOpenGLES2(EAGLContextWrapper* context)
:	m_context(context)
{
	std::memset(&m_renderState, 0, sizeof(m_renderState));
}
#elif defined(T_OPENGL_ES2_HAVE_EGL)
ContextOpenGLES2::ContextOpenGLES2(EGLDisplay display, EGLSurface surface, EGLConfig context)
:	m_display(display)
,	m_surface(surface)
,	m_context(context)
{
	std::memset(&m_renderState, 0, sizeof(m_renderState));
}
#endif

	}
}
