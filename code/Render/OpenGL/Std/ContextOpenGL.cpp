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
,	m_currentStateList(0)
,	m_width(0)
,	m_height(0)
,	m_fullscreen(false)

#elif defined(__APPLE__)

ContextOpenGL::ContextOpenGL(void* context, bool fullscreen)
:	m_context(context)
,	m_currentStateList(0)
,	m_width(0)
,	m_height(0)
,	m_fullscreen(fullscreen)

#else	// LINUX

ContextOpenGL::ContextOpenGL(Display* display, Window window, GLXContext context)
:	m_display(display)
,	m_window(window)
,	m_context(context)
,	m_currentStateList(0)
,	m_width(0)
,	m_height(0)
,	m_fullscreen(false)

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

void ContextOpenGL::swapBuffers()
{
#if defined(_WIN32)
	SwapBuffers(m_hDC);
#elif defined(__APPLE__)
	cglwSwapBuffers(m_context);
#else	// LINUX
	glXSwapBuffers(m_display, m_window);
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

bool ContextOpenGL::isFullScreen() const
{
	return m_fullscreen;
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

	if (m_window)
		glXMakeCurrent(
			m_display,
			m_window,
			m_context
		);

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

#endif

	m_lock.release();
}

void ContextOpenGL::enable(GLenum state)
{
	if (!m_enableStates[state])
	{
		T_OGL_SAFE(glEnable(state));
		m_enableStates[state] = true;
	}
}

void ContextOpenGL::disable(GLenum state)
{
	if (m_enableStates[state])
	{
		T_OGL_SAFE(glDisable(state));
		m_enableStates[state] = false;
	}
}

GLuint ContextOpenGL::createStateList(const RenderState& renderState)
{
	Adler32 adler;
	
	adler.begin();
	adler.feed(&renderState, sizeof(renderState));
	adler.end();
	
	uint32_t hash = adler.get();
	
	std::map< uint32_t, GLuint >::const_iterator i = m_stateLists.find(hash);
	if (i != m_stateLists.end())
		return i->second;
		
	GLuint stateList = glGenLists(1);
	glNewList(stateList, GL_COMPILE);

	if (renderState.cullFaceEnable)
	{
		T_OGL_SAFE(glEnable(GL_CULL_FACE));
		T_OGL_SAFE(glCullFace(renderState.cullFace));
	}
	else
		T_OGL_SAFE(glDisable(GL_CULL_FACE));

	if (renderState.blendEnable)
	{
		T_OGL_SAFE(glEnable(GL_BLEND));
		T_OGL_SAFE(glBlendFunc(renderState.blendFuncSrc, renderState.blendFuncDest));
		T_OGL_SAFE(glBlendEquationEXT(renderState.blendEquation));
	}
	else
		T_OGL_SAFE(glDisable(GL_BLEND));

	if (renderState.depthTestEnable)
	{
		T_OGL_SAFE(glEnable(GL_DEPTH_TEST));
		T_OGL_SAFE(glDepthFunc(renderState.depthFunc));
	}
	else
		T_OGL_SAFE(glDisable(GL_DEPTH_TEST));

	T_OGL_SAFE(glColorMask(
		(renderState.colorMask & RenderState::CmRed) ? GL_TRUE : GL_FALSE,
		(renderState.colorMask & RenderState::CmGreen) ? GL_TRUE : GL_FALSE,
		(renderState.colorMask & RenderState::CmBlue) ? GL_TRUE : GL_FALSE,
		(renderState.colorMask & RenderState::CmAlpha) ? GL_TRUE : GL_FALSE
	));

	T_OGL_SAFE(glDepthMask(renderState.depthMask));

	if (renderState.alphaTestEnable)
	{
		T_OGL_SAFE(glEnable(GL_ALPHA_TEST));
		T_OGL_SAFE(glAlphaFunc(renderState.alphaFunc, renderState.alphaRef));
	}
	else
		T_OGL_SAFE(glDisable(GL_ALPHA_TEST));
		
	if (renderState.stencilTestEnable)
	{
		T_OGL_SAFE(glEnable(GL_STENCIL_TEST));
		T_OGL_SAFE(glStencilFunc(renderState.stencilFunc, renderState.stencilRef, ~0UL));
		T_OGL_SAFE(glStencilOp(
			renderState.stencilOpFail,
			renderState.stencilOpZFail,
			renderState.stencilOpZPass
		));
	}
	else
		T_OGL_SAFE(glDisable(GL_STENCIL_TEST));

	glEndList();
	
	m_stateLists.insert(std::make_pair(hash, stateList));
	return stateList;
}

void ContextOpenGL::callStateList(GLuint stateList)
{
	if (m_currentStateList != stateList)
	{
		T_OGL_SAFE(glCallList(stateList));
		m_currentStateList = stateList;
	}
}

GLhandleARB ContextOpenGL::createShaderObject(const char* shader, GLenum shaderType)
{
	char errorBuf[32000];
	GLsizei errorBufLen;
	GLint status;
	
	Adler32 adler;
	adler.begin();
	adler.feed(shader, strlen(shader));
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
