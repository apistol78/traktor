#include <cstring>
#include "Core/RefArray.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/TString.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/ThreadManager.h"
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

void APIENTRY debugCallbackARB(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, GLvoid *userParam)
{
#if !defined(_DEBUG)
	if (severity == GL_DEBUG_SEVERITY_LOW_ARB)
		return;
#endif

	const wchar_t* s = L"unknown";

	if (severity == GL_DEBUG_SEVERITY_LOW_ARB)
		s = L"low";
	else if (severity == GL_DEBUG_SEVERITY_MEDIUM_ARB)
		s = L"medium";
	else if (severity == GL_DEBUG_SEVERITY_HIGH_ARB)
		s = L"high";

	if (message)
		log::info << L"OpenGL (" << s << L"): " << mbstows(message) << Endl;
	else
		log::info << L"OpenGL (" << s << L"): <empty>" << Endl;
}

void APIENTRY debugCallbackAMD(GLuint id, GLenum category, GLenum severity, GLsizei length, const GLchar *message, GLvoid *userParam)
{
	if (message)
		log::info << L"OpenGL: " << mbstows(message) << Endl;
	else
		log::info << L"OpenGL: <empty>" << Endl;
}

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
,	m_permitDepth(true)
,	m_currentList(0)

#elif defined(__APPLE__)

ContextOpenGL::ContextOpenGL(void* context)
:	m_context(context)
,	m_width(0)
,	m_height(0)
,	m_permitDepth(true)
,	m_currentList(0)

#elif defined(__LINUX__)

ContextOpenGL::ContextOpenGL(Display* display, GLXDrawable drawable, GLXContext context)
:	m_display(display)
,	m_drawable(drawable)
,	m_context(context)
,	m_width(0)
,	m_height(0)
,	m_permitDepth(true)
,	m_currentList(0)

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
#elif defined(__LINUX__)
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
#elif defined(__LINUX__)
	glXQueryDrawable(m_display, m_drawable, GLX_WIDTH, (uint32_t*)&m_width);
	glXQueryDrawable(m_display, m_drawable, GLX_HEIGHT, (uint32_t*)&m_height);
#endif
}

void ContextOpenGL::swapBuffers(bool waitVBlank)
{
#if defined(_WIN32)
	SwapBuffers(m_hDC);
#elif defined(__APPLE__)
	cglwSwapBuffers(m_context, waitVBlank);
#elif defined(__LINUX__)
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

#elif defined(__LINUX__)

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
#elif defined(__LINUX__)
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

	if (glDebugMessageCallbackARB)
	{
		T_OGL_SAFE(glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE));
		T_OGL_SAFE(glDebugMessageCallbackARB(&debugCallbackARB, 0));
#if defined(_WIN32)
#	if defined(_DEBUG)
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#	else
		glEnable(GL_DEBUG_OUTPUT);
#	endif
#endif
	}
	if (glDebugMessageCallbackAMD)
	{
		T_OGL_SAFE(glDebugMessageEnableAMD(0, 0, 0, NULL, GL_TRUE));
		T_OGL_SAFE(glDebugMessageCallbackAMD(&debugCallbackAMD, 0));
	}

	stack->push_back(this);
	return true;
}

void ContextOpenGL::leave()
{
	context_stack_t* stack = static_cast< context_stack_t* >(ms_contextStack.get());
	bool result = true;

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

#elif defined(__LINUX__)

	if (!stack->empty())
	{
		result = (glXMakeContextCurrent(
			m_display,
			stack->back()->m_drawable,
			stack->back()->m_drawable,
			stack->back()->m_context
		) == True);
	}
	else
	{
		result = (glXMakeContextCurrent(m_display, None, None, NULL) == True);
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

GLuint ContextOpenGL::createStateList(const RenderState& renderState)
{
	Adler32 adler;
	adler.feed(renderState.cullFaceEnable);
	adler.feed(renderState.cullFace);
	adler.feed(renderState.blendEnable);
	adler.feed(renderState.blendEquation);
	adler.feed(renderState.blendFuncSrc);
	adler.feed(renderState.blendFuncDest);
	adler.feed(renderState.depthTestEnable);
	adler.feed(renderState.colorMask);
	adler.feed(renderState.depthMask);
	adler.feed(renderState.depthFunc);
	adler.feed(renderState.alphaTestEnable);
	adler.feed(renderState.alphaFunc);
	adler.feed(renderState.alphaRef);
	adler.feed(renderState.stencilTestEnable);
	adler.feed(renderState.stencilOpFail);
	adler.feed(renderState.stencilOpZFail);
	adler.feed(renderState.stencilOpZPass);

	std::map< uint32_t, GLuint >::iterator i = m_stateLists.find(adler.get());
	if (i != m_stateLists.end())
		return i->second;

	GLuint listBase = glGenLists(2);

	for (uint32_t i = 0; i < 2; ++i)
	{
		bool permitDepth = bool(i == 0);
		bool invertCull = true;

		RenderState rs = renderState;
		if (!permitDepth)
		{
			rs.depthTestEnable = GL_FALSE;
			rs.depthMask = GL_FALSE;
			rs.stencilTestEnable = GL_FALSE;
		}

		if (invertCull)
		{
			if (rs.cullFace == GL_FRONT)
				rs.cullFace = GL_BACK;
			else
				rs.cullFace = GL_FRONT;
		}

		glNewList(listBase + i, GL_COMPILE);

		if (rs.cullFaceEnable)
			{ T_OGL_SAFE(glEnable(GL_CULL_FACE)); }
		else
			{ T_OGL_SAFE(glDisable(GL_CULL_FACE)); }

		T_OGL_SAFE(glCullFace(rs.cullFace));

		if (rs.blendEnable)
			{ T_OGL_SAFE(glEnable(GL_BLEND)); }
		else
			{ T_OGL_SAFE(glDisable(GL_BLEND)); }

		T_OGL_SAFE(glBlendFunc(rs.blendFuncSrc, rs.blendFuncDest));
		T_OGL_SAFE(glBlendEquationEXT(rs.blendEquation));

		if (rs.depthTestEnable && permitDepth)
			{ T_OGL_SAFE(glEnable(GL_DEPTH_TEST)); }
		else
			{ T_OGL_SAFE(glDisable(GL_DEPTH_TEST)); }

		T_OGL_SAFE(glDepthFunc(rs.depthFunc));

		if (permitDepth)
			{ T_OGL_SAFE(glDepthMask(rs.depthMask)); }
		else
			{ T_OGL_SAFE(glDepthMask(GL_FALSE)); }

		T_OGL_SAFE(glColorMask(
			(rs.colorMask & RenderState::CmRed) ? GL_TRUE : GL_FALSE,
			(rs.colorMask & RenderState::CmGreen) ? GL_TRUE : GL_FALSE,
			(rs.colorMask & RenderState::CmBlue) ? GL_TRUE : GL_FALSE,
			(rs.colorMask & RenderState::CmAlpha) ? GL_TRUE : GL_FALSE
		));

		if (rs.stencilTestEnable)
			{ T_OGL_SAFE(glEnable(GL_STENCIL_TEST)); }
		else
			{ T_OGL_SAFE(glDisable(GL_STENCIL_TEST)); }

		T_OGL_SAFE(glStencilMask(~0UL));
		T_OGL_SAFE(glStencilOp(rs.stencilOpFail, rs.stencilOpZFail, rs.stencilOpZPass));

		glEndList();
	}

	m_stateLists[adler.get()] = listBase;
	return listBase;
}

void ContextOpenGL::callStateList(GLuint listBase)
{
	GLuint list = listBase + (m_permitDepth ? 0 : 1);
	if (list != m_currentList)
	{
		glCallList(list);
		m_currentList = list;
	}
}

void ContextOpenGL::setPermitDepth(bool permitDepth)
{
	m_permitDepth = permitDepth;
}

void ContextOpenGL::deleteResource(IDeleteCallback* callback)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_deleteResources.push_back(callback);
}

void ContextOpenGL::deleteResources()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	if (!m_deleteResources.empty())
	{
		for (std::vector< IDeleteCallback* >::iterator i = m_deleteResources.begin(); i != m_deleteResources.end(); ++i)
			(*i)->deleteResource();
		m_deleteResources.resize(0);
	}
}

	}
}
