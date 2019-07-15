#include "Render/OpenGL/Std/RenderContextOpenGL.h"
#include "Render/OpenGL/Std/ResourceContextOpenGL.h"

#if defined(__APPLE__)
#	include "Render/OpenGL/Std/OsX/CGLWrapper.h"
#endif

namespace traktor
{
	namespace render
	{
		namespace
		{
		
const GLenum c_oglCullFace[] =
{
	GL_FRONT,	// Never, culling is disabled.
	GL_FRONT,
	GL_BACK
};

const GLenum c_oglBlendEquation[] =
{
	GL_FUNC_ADD,
	GL_FUNC_SUBTRACT,
	GL_FUNC_REVERSE_SUBTRACT,
	GL_MIN,
	GL_MAX
};

const GLenum c_oglBlendFunction[] =
{
	GL_ONE,
	GL_ZERO,
	GL_SRC_COLOR,
	GL_ONE_MINUS_SRC_COLOR,
	GL_DST_COLOR,
	GL_ONE_MINUS_DST_COLOR,
	GL_SRC_ALPHA,
	GL_ONE_MINUS_SRC_ALPHA,
	GL_DST_ALPHA,
	GL_ONE_MINUS_DST_ALPHA
};

const GLenum c_oglFunction[] =
{
	GL_ALWAYS,
	GL_NEVER,
	GL_LESS,
	GL_LEQUAL,
	GL_GREATER,
	GL_GEQUAL,
	GL_EQUAL,
	GL_NOTEQUAL
};

const GLenum c_oglStencilOperation[] =
{
	GL_KEEP,
	GL_ZERO,
	GL_REPLACE,
	GL_INCR,
	GL_DECR,
	GL_INVERT,
	GL_INCR_WRAP,
	GL_DECR_WRAP
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderContextOpenGL", RenderContextOpenGL, ContextOpenGL)

#if defined(_WIN32)
RenderContextOpenGL::RenderContextOpenGL(ResourceContextOpenGL* resourceContext, HWND hWnd, HDC hDC, HGLRC hRC)
:	ContextOpenGL(hWnd, hDC, hRC)
#elif defined(__APPLE__)
RenderContextOpenGL::RenderContextOpenGL(ResourceContextOpenGL* resourceContext, void* context)
:	ContextOpenGL(context)
#elif defined(__LINUX__)
RenderContextOpenGL::RenderContextOpenGL(ResourceContextOpenGL* resourceContext, ::Display* display, ::Window window, GLXContext context)
:	ContextOpenGL(display, window, context)
#endif
,	m_resourceContext(resourceContext)
,	m_width(0)
,	m_height(0)
,	m_permitDepth(true)
,	m_currentProgram(nullptr)
,	m_currentRenderStateList(~0U)
,	m_lastWaitVBlanks(-1)
{
#if !defined(__LINUX__)
	update();
#endif
}

#if !defined(__LINUX__)
void RenderContextOpenGL::update()
#else
void RenderContextOpenGL::update(int32_t width, int32_t height)
#endif
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
	m_width = width;
	m_height = height;
#endif
}

void RenderContextOpenGL::swapBuffers(int32_t waitVBlanks)
{
#if defined(_WIN32)
	if (m_lastWaitVBlanks != waitVBlanks)
	{
		if (wglSwapIntervalEXT)
			wglSwapIntervalEXT(waitVBlanks);
	}
	SwapBuffers(m_hDC);
#elif defined(__APPLE__)
	cglwSwapBuffers(m_context, waitVBlanks);
#elif defined(__LINUX__)
	if (m_lastWaitVBlanks != waitVBlanks)
	{
		if (glXSwapIntervalEXT != 0)
		{
			unsigned int current = 0;
			glXQueryDrawable(m_display, m_window, GLX_SWAP_INTERVAL_EXT, &current);
			glXSwapIntervalEXT(m_display, m_window, current);
			glXSwapIntervalEXT(m_display, m_window, waitVBlanks);
		}
	}
	glXSwapBuffers(m_display, m_window);
#endif
	m_lastWaitVBlanks = waitVBlanks;
}

bool RenderContextOpenGL::programActivate(const ProgramOpenGL* program)
{
	bool alreadyActive = bool(program == m_currentProgram);
	m_currentProgram = program;
	return alreadyActive;
}

void RenderContextOpenGL::bindRenderStateObject(uint32_t renderStateObject, uint32_t stencilReference)
{
	const AlignedVector< RenderState >& renderStateList = m_resourceContext->getRenderStateList();
	T_ASSERT(renderStateObject > 0);
	T_ASSERT(renderStateObject <= renderStateList.size());
	const RenderState& rs = renderStateList[renderStateObject - 1];

	if (renderStateObject == m_currentRenderStateList)
	{
		// Only ensure stencil reference is properly updated.
		if (rs.stencilEnable)
		{
	 		T_OGL_SAFE(glStencilFunc(
	 			c_oglFunction[rs.stencilFunction],
	 			stencilReference,
	 			~0U
	 		));
		}
		return;
	}

	if (rs.cullMode != CmNever)
	 	{ T_OGL_SAFE(glEnable(GL_CULL_FACE)); }
	else
		{ T_OGL_SAFE(glDisable(GL_CULL_FACE)); }

	T_OGL_SAFE(glCullFace(c_oglCullFace[rs.cullMode]));

	if (rs.blendEnable)
	 	{ T_OGL_SAFE(glEnable(GL_BLEND)); }
	else
	 	{ T_OGL_SAFE(glDisable(GL_BLEND)); }

	T_OGL_SAFE(glBlendFuncSeparate(
		 c_oglBlendFunction[rs.blendColorSource],
		 c_oglBlendFunction[rs.blendColorDestination],
		 c_oglBlendFunction[rs.blendAlphaSource],
		 c_oglBlendFunction[rs.blendAlphaDestination]
	));
	T_OGL_SAFE(glBlendEquationSeparate(
		 c_oglBlendEquation[rs.blendColorOperation],
		 c_oglBlendEquation[rs.blendAlphaOperation]
	));

	if (rs.depthEnable && m_permitDepth)
	 	{ T_OGL_SAFE(glEnable(GL_DEPTH_TEST)); }
	else
	 	{ T_OGL_SAFE(glDisable(GL_DEPTH_TEST)); }

	T_OGL_SAFE(glDepthFunc(c_oglFunction[rs.depthFunction]));
	T_OGL_SAFE(glDepthMask(rs.depthWriteEnable ? GL_TRUE : GL_FALSE));

	T_OGL_SAFE(glColorMask(
	 	(rs.colorWriteMask & CwRed) ? GL_TRUE : GL_FALSE,
	 	(rs.colorWriteMask & CwGreen) ? GL_TRUE : GL_FALSE,
	 	(rs.colorWriteMask & CwBlue) ? GL_TRUE : GL_FALSE,
	 	(rs.colorWriteMask & CwAlpha) ? GL_TRUE : GL_FALSE
	 ));

	if (rs.stencilEnable)
		{ T_OGL_SAFE(glEnable(GL_STENCIL_TEST)); }
	else
	 	{ T_OGL_SAFE(glDisable(GL_STENCIL_TEST)); }

	T_OGL_SAFE(glStencilMask(~0U));
	T_OGL_SAFE(glStencilOp(
		 c_oglStencilOperation[rs.stencilFail],
		 c_oglStencilOperation[rs.stencilZFail],
		 c_oglStencilOperation[rs.stencilPass]
	));

	T_OGL_SAFE(glStencilFunc(
		c_oglFunction[rs.stencilFunction],
		stencilReference,
		~0U
	));

	m_currentRenderStateList = renderStateObject;
}

void RenderContextOpenGL::bindSamplerStateObject(uint32_t samplerStateObject, uint32_t stage, bool haveMips)
{
	const auto& samplerStateObjects = m_resourceContext->getSamplerStateObjects();
	
	auto it = samplerStateObjects.find(samplerStateObject);
	T_FATAL_ASSERT(it != samplerStateObjects.end());

	if (haveMips)
	{
		T_ASSERT(glIsSampler(it->second.samplers[0]) == GL_TRUE);
		T_OGL_SAFE(glBindSampler(stage, it->second.samplers[0]));
	}
	else
	{
		T_ASSERT(glIsSampler(it->second.samplers[1]) == GL_TRUE);
		T_OGL_SAFE(glBindSampler(stage, it->second.samplers[1]));
	}
}

bool RenderContextOpenGL::bindVertexArrayObject(uint32_t vertexBufferId)
{
	auto it = m_vertexArrayObjects.find(vertexBufferId);
	if (it != m_vertexArrayObjects.end())
	{
		T_ASSERT(glIsVertexArray(it->second) == GL_TRUE);
		T_OGL_SAFE(glBindVertexArray(it->second));
		return true;
	}
	else
	{
		GLuint vertexArrayObject;
		T_OGL_SAFE(glGenVertexArrays(1, &vertexArrayObject));
		T_OGL_SAFE(glBindVertexArray(vertexArrayObject));
		m_vertexArrayObjects.insert(vertexBufferId, vertexArrayObject);
		return false;
	}
}

void RenderContextOpenGL::setPermitDepth(bool permitDepth)
{
	m_permitDepth = permitDepth;
	m_currentRenderStateList = ~0U;
}

	}
}
