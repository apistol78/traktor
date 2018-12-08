/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/OpenGL/Std/RenderContextOpenGL.h"
#include "Render/OpenGL/Std/ResourceContextOpenGL.h"

#if defined(__APPLE__)
#	include "Render/OpenGL/Std/OsX/CGLWrapper.h"
#endif

namespace traktor
{
	namespace render
	{

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

void RenderContextOpenGL::bindRenderStateObject(uint32_t renderStateObject)
{
	renderStateObject += (m_permitDepth ? 0 : 1);
	if (renderStateObject == m_currentRenderStateList)
		return;

	const AlignedVector< RenderStateOpenGL >& renderStateList = m_resourceContext->getRenderStateList();

	T_ASSERT (renderStateObject > 0);
	T_ASSERT (renderStateObject <= renderStateList.size());
	const RenderStateOpenGL& rs = renderStateList[renderStateObject - 1];

	if (rs.cullFaceEnable)
		{ T_OGL_SAFE(glEnable(GL_CULL_FACE)); }
	else
		{ T_OGL_SAFE(glDisable(GL_CULL_FACE)); }

	T_OGL_SAFE(glCullFace(rs.cullFace));

	if (rs.blendEnable)
		{ T_OGL_SAFE(glEnable(GL_BLEND)); }
	else
		{ T_OGL_SAFE(glDisable(GL_BLEND)); }

	T_OGL_SAFE(glBlendFuncSeparate(rs.blendFuncColorSrc, rs.blendFuncColorDest, rs.blendFuncAlphaSrc, rs.blendFuncAlphaDest));
	T_OGL_SAFE(glBlendEquationSeparate(rs.blendColorEquation, rs.blendAlphaEquation));

	if (rs.depthTestEnable)
		{ T_OGL_SAFE(glEnable(GL_DEPTH_TEST)); }
	else
		{ T_OGL_SAFE(glDisable(GL_DEPTH_TEST)); }

	T_OGL_SAFE(glDepthFunc(rs.depthFunc));
	T_OGL_SAFE(glDepthMask(rs.depthMask));

	T_OGL_SAFE(glColorMask(
		(rs.colorMask & RenderStateOpenGL::CmRed) ? GL_TRUE : GL_FALSE,
		(rs.colorMask & RenderStateOpenGL::CmGreen) ? GL_TRUE : GL_FALSE,
		(rs.colorMask & RenderStateOpenGL::CmBlue) ? GL_TRUE : GL_FALSE,
		(rs.colorMask & RenderStateOpenGL::CmAlpha) ? GL_TRUE : GL_FALSE
	));

	if (rs.stencilTestEnable)
		{ T_OGL_SAFE(glEnable(GL_STENCIL_TEST)); }
	else
		{ T_OGL_SAFE(glDisable(GL_STENCIL_TEST)); }

	T_OGL_SAFE(glStencilMask(~0U));
	T_OGL_SAFE(glStencilOp(rs.stencilOpFail, rs.stencilOpZFail, rs.stencilOpZPass));

	m_currentRenderStateList = renderStateObject;
}

void RenderContextOpenGL::bindSamplerStateObject(uint32_t samplerStateObject, uint32_t stage, bool haveMips)
{
	auto it = m_resourceContext->getSamplerStateObjects().find(samplerStateObject);
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
}

	}
}
