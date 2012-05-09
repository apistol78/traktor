#include <cstring>
#include "Core/RefArray.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/TString.h"
#include "Core/Thread/Acquire.h"
#include "Render/OpenGL/Std/ContextOpenGL.h"
#include "Render/OpenGL/Std/Extensions.h"

#if defined(__APPLE__)
#	include "Render/OpenGL/Std/OsX/CGLWrapper.h"
#endif

namespace traktor
{
	namespace render
	{
		namespace
		{

typedef RefArray< ContextOpenGL > context_stack_t;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ContextOpenGL", ContextOpenGL, IContext)

ThreadLocal ContextOpenGL::ms_contextStack;

#if defined(_WIN32)

ContextOpenGL::ContextOpenGL(HWND hWnd, HDC hDC, HGLRC hRC)
:	m_hWnd(hWnd)
,	m_hDC(hDC)
,	m_hRC(hRC)
,	m_width(0)
,	m_height(0)

#elif defined(__APPLE__)

ContextOpenGL::ContextOpenGL(void* context)
:	m_context(context)
,	m_width(0)
,	m_height(0)

#else	// LINUX

ContextOpenGL::ContextOpenGL(Display* display, GLXDrawable drawable, GLXContext context)
:	m_display(display)
,	m_drawable(drawable)
,	m_context(context)
,	m_width(0)
,	m_height(0)

#endif
{
	update();
}

ContextOpenGL::~ContextOpenGL()
{
#if defined(_WIN32)
	T_ASSERT (!m_hRC);
#elif defined(__APPLE__)
	T_ASSERT (!m_context);
#else	// LINUX
	T_ASSERT (!m_context);
#endif
}

void ContextOpenGL::share(ContextOpenGL* context)
{
#if defined(_WIN32)
	wglShareLists(context->m_hRC, m_hRC);
	wglShareLists(m_hRC, context->m_hRC);
#endif
}

void ContextOpenGL::update()
{
#if defined(_WIN32)
	RECT rc;
	GetClientRect(m_hWnd, &rc);
	m_width = int32_t(rc.right - rc.left);
	m_height = int32_t(rc.bottom - rc.top);
#elif defined(__APPLE__)
	cglwUpdate(m_context);
	cglwGetSize(m_context, m_width, m_height);
#endif
}

void ContextOpenGL::swapBuffers(bool waitVBlank)
{
#if defined(_WIN32)
	SwapBuffers(m_hDC);
#elif defined(__APPLE__)
	cglwSwapBuffers(m_context, waitVBlank);
#else	// LINUX
	glXSwapBuffers(m_display, m_drawable);
#endif
}

void ContextOpenGL::destroy()
{
#if defined(_WIN32)

	if (m_hRC)
	{
		wglMakeCurrent(0, 0);
		wglDeleteContext(m_hRC);

		ReleaseDC(m_hWnd, m_hDC);

		m_hWnd = 0;
		m_hDC = 0;
		m_hRC = 0;
	}

#elif defined(__APPLE__)

	if (m_context)
		m_context = 0;

#else	// LINUX

	if (m_context)
	{
		glXDestroyContext(m_display, m_context);
		m_context = 0;
	}

#endif
}

int32_t ContextOpenGL::getWidth() const
{
	return m_width;
}

int32_t ContextOpenGL::getHeight() const
{
	return m_height;
}

bool ContextOpenGL::enter()
{
	if (!m_lock.wait())
		return false;

#if defined(_WIN32)

	if (!wglMakeCurrent(m_hDC, m_hRC))
		return false;

#elif defined(__APPLE__)

	if (!cglwMakeCurrent(m_context))
		return false;

#else	// LINUX

	if (m_drawable)
	{
		if (!glXMakeCurrent(
			m_display,
			m_drawable,
			m_context
		))
			return false;
	}

#endif

	context_stack_t* stack = static_cast< context_stack_t* >(ms_contextStack.get());
	if (!stack)
	{
		stack = new context_stack_t();
		ms_contextStack.set(stack);
	}

	stack->push_back(this);

	return true;
}

void ContextOpenGL::leave()
{
	context_stack_t* stack = static_cast< context_stack_t* >(ms_contextStack.get());

	T_ASSERT (stack);
	T_ASSERT (!stack->empty());
	T_ASSERT (stack->back() == this);

	stack->pop_back();

#if defined(_WIN32)

	if (!stack->empty())
	{
		wglMakeCurrent(
			stack->back()->m_hDC,
			stack->back()->m_hRC
		);
	}
	else
		wglMakeCurrent(m_hDC, NULL);

#elif defined(__APPLE__)

	if (!stack->empty())
		cglwMakeCurrent(stack->back()->m_context);
	else
		cglwMakeCurrent(0);

#else	// LINUX

	if (!stack->empty())
	{
		glXMakeCurrent(
			stack->back()->m_display,
			stack->back()->m_drawable,
			stack->back()->m_context
		);
	}
	else
	{
		Display* display = XOpenDisplay(0);
		T_ASSERT (display);

		glXMakeCurrent(display, None, NULL);
	}

#endif

	m_lock.release();
}

GLhandleARB ContextOpenGL::createShaderObject(const char* shader, GLenum shaderType)
{
	char errorBuf[32000];
	GLsizei errorBufLen;
	GLint status;

	Adler32 adler;
	adler.begin();
	adler.feed(shader, std::strlen(shader));
	adler.end();

	uint32_t hash = adler.get();

	std::map< uint32_t, GLhandleARB >::const_iterator i = m_shaderObjects.find(hash);
	if (i != m_shaderObjects.end())
		return i->second;

	GLhandleARB shaderObject = glCreateShaderObjectARB(shaderType);
	T_OGL_SAFE(glShaderSourceARB(shaderObject, 1, &shader, NULL));
	T_OGL_SAFE(glCompileShaderARB(shaderObject));

	T_OGL_SAFE(glGetObjectParameterivARB(shaderObject, GL_OBJECT_COMPILE_STATUS_ARB, &status));
	if (status != 1)
	{
		T_OGL_SAFE(glGetInfoLogARB(shaderObject, sizeof(errorBuf), &errorBufLen, errorBuf));
		if (errorBufLen > 0)
		{
			log::error << L"GLSL shader compile failed :" << Endl;
			log::error << mbstows(errorBuf) << Endl;
			log::error << Endl;
			FormatMultipleLines(log::error, mbstows(shader));
			return 0;
		}
	}

	m_shaderObjects.insert(std::make_pair(hash, shaderObject));
	return shaderObject;
}

void ContextOpenGL::deleteResource(IDeleteCallback* callback)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_deleteResources.push_back(callback);
}

void ContextOpenGL::deleteResources()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	for (std::vector< IDeleteCallback* >::iterator i = m_deleteResources.begin(); i != m_deleteResources.end(); ++i)
		(*i)->deleteResource();
	m_deleteResources.resize(0);
}

	}
}
