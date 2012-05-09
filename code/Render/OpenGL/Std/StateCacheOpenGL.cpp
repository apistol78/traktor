#include "Render/OpenGL/Std/Extensions.h"
#include "Render/OpenGL/Std/StateCacheOpenGL.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.StateCacheOpenGL", StateCacheOpenGL, Object)

StateCacheOpenGL::StateCacheOpenGL()
:	m_permitDepth(true)
{
}

void StateCacheOpenGL::forceRenderState(const RenderState& renderState, bool invertCull)
{
	if (renderState.cullFaceEnable)
		{ T_OGL_SAFE(glEnable(GL_CULL_FACE)); }
	else
		{ T_OGL_SAFE(glDisable(GL_CULL_FACE)); }

	GLuint cullFace = renderState.cullFace;
	if (invertCull)
	{
		if (cullFace == GL_FRONT)
			cullFace = GL_BACK;
		else
			cullFace = GL_FRONT;
	}

	T_OGL_SAFE(glCullFace(cullFace));

	if (renderState.blendEnable)
		{ T_OGL_SAFE(glEnable(GL_BLEND)); }
	else
		{ T_OGL_SAFE(glDisable(GL_BLEND)); }

	T_OGL_SAFE(glBlendFunc(renderState.blendFuncSrc, renderState.blendFuncDest));
	T_OGL_SAFE(glBlendEquationEXT(renderState.blendEquation));

	if (renderState.depthTestEnable)
		{ T_OGL_SAFE(glEnable(GL_DEPTH_TEST)); }
	else
		{ T_OGL_SAFE(glDisable(GL_DEPTH_TEST)); }

	T_OGL_SAFE(glDepthFunc(renderState.depthFunc));

	T_OGL_SAFE(glColorMask(
		(renderState.colorMask & RenderState::CmRed) ? GL_TRUE : GL_FALSE,
		(renderState.colorMask & RenderState::CmGreen) ? GL_TRUE : GL_FALSE,
		(renderState.colorMask & RenderState::CmBlue) ? GL_TRUE : GL_FALSE,
		(renderState.colorMask & RenderState::CmAlpha) ? GL_TRUE : GL_FALSE
	));

	T_OGL_SAFE(glDepthMask(renderState.depthMask));

	if (renderState.stencilTestEnable)
		{ T_OGL_SAFE(glEnable(GL_STENCIL_TEST)); }
	else
		{ T_OGL_SAFE(glDisable(GL_STENCIL_TEST)); }

	T_OGL_SAFE(glStencilFunc(renderState.stencilFunc, renderState.stencilRef, ~0UL));

	m_shadowRenderState = renderState;
}

void StateCacheOpenGL::setRenderState(const RenderState& renderState, bool invertCull)
{
	if (renderState.cullFaceEnable)
	{
		GLuint cullFace = renderState.cullFace;
		if (invertCull)
		{
			if (cullFace == GL_FRONT)
				cullFace = GL_BACK;
			else
				cullFace = GL_FRONT;
		}

		if (!m_shadowRenderState.cullFaceEnable)
		{
			T_OGL_SAFE(glEnable(GL_CULL_FACE));
			m_shadowRenderState.cullFaceEnable = GL_TRUE;
		}

		if (cullFace != m_shadowRenderState.cullFace)
		{
			T_OGL_SAFE(glCullFace(cullFace));
			m_shadowRenderState.cullFace = cullFace;
		}
	}
	else
	{
		if (m_shadowRenderState.cullFaceEnable)
		{
			T_OGL_SAFE(glDisable(GL_CULL_FACE));
			m_shadowRenderState.cullFaceEnable = GL_FALSE;
		}
	}

	if (renderState.blendEnable)
	{
		if (!m_shadowRenderState.blendEnable)
		{
			T_OGL_SAFE(glEnable(GL_BLEND));
			m_shadowRenderState.blendEnable = GL_TRUE;
		}

		if (renderState.blendFuncSrc != m_shadowRenderState.blendFuncSrc || renderState.blendFuncDest != m_shadowRenderState.blendFuncDest)
		{
			T_OGL_SAFE(glBlendFunc(renderState.blendFuncSrc, renderState.blendFuncDest));
			m_shadowRenderState.blendFuncSrc = renderState.blendFuncSrc;
			m_shadowRenderState.blendFuncDest = renderState.blendFuncDest;
		}

		if (renderState.blendEquation != m_shadowRenderState.blendEquation)
		{
			T_OGL_SAFE(glBlendEquationEXT(renderState.blendEquation));
			m_shadowRenderState.blendEquation = renderState.blendEquation;
		}
	}
	else
	{
		if (m_shadowRenderState.blendEnable)
		{
			T_OGL_SAFE(glDisable(GL_BLEND));
			m_shadowRenderState.blendEnable = GL_FALSE;
		}
	}

	if (renderState.depthTestEnable && m_permitDepth)
	{
		if (!m_shadowRenderState.depthTestEnable)
		{
			T_OGL_SAFE(glEnable(GL_DEPTH_TEST));
			m_shadowRenderState.depthTestEnable = GL_TRUE;
		}
		
		if (renderState.depthFunc != m_shadowRenderState.depthFunc)
		{
			T_OGL_SAFE(glDepthFunc(renderState.depthFunc));
			m_shadowRenderState.depthFunc = renderState.depthFunc;
		}
	}
	else
	{
		if (m_shadowRenderState.depthTestEnable)
		{
			T_OGL_SAFE(glDisable(GL_DEPTH_TEST));
			m_shadowRenderState.depthTestEnable = GL_FALSE;
		}
	}

	if (renderState.colorMask != m_shadowRenderState.colorMask)
	{
		T_OGL_SAFE(glColorMask(
			(renderState.colorMask & RenderState::CmRed) ? GL_TRUE : GL_FALSE,
			(renderState.colorMask & RenderState::CmGreen) ? GL_TRUE : GL_FALSE,
			(renderState.colorMask & RenderState::CmBlue) ? GL_TRUE : GL_FALSE,
			(renderState.colorMask & RenderState::CmAlpha) ? GL_TRUE : GL_FALSE
		));
		m_shadowRenderState.colorMask = renderState.colorMask;
	}

	if (renderState.depthMask != m_shadowRenderState.depthMask)
	{
		T_OGL_SAFE(glDepthMask(renderState.depthMask));
		m_shadowRenderState.depthMask = renderState.depthMask;
	}

	if (renderState.stencilTestEnable && m_permitDepth)
	{
		if (!m_shadowRenderState.stencilTestEnable)
		{
			T_OGL_SAFE(glEnable(GL_STENCIL_TEST));
			m_shadowRenderState.stencilTestEnable = GL_TRUE;
		}

		if (
			renderState.stencilFunc != m_shadowRenderState.stencilFunc ||
			renderState.stencilRef != m_shadowRenderState.stencilRef
		)
		{
			T_OGL_SAFE(glStencilFunc(renderState.stencilFunc, renderState.stencilRef, ~0UL));
			m_shadowRenderState.stencilFunc = renderState.stencilFunc;
			m_shadowRenderState.stencilRef = renderState.stencilRef;
		}
	}
	else
	{
		if (m_shadowRenderState.stencilTestEnable)
		{
			T_OGL_SAFE(glDisable(GL_STENCIL_TEST));
			m_shadowRenderState.stencilTestEnable = GL_FALSE;
		}
	}
}

void StateCacheOpenGL::setColorMask(uint32_t colorMask)
{
	if (colorMask != m_shadowRenderState.colorMask)
	{
		T_OGL_SAFE(glColorMask(
			(colorMask & RenderState::CmRed) ? GL_TRUE : GL_FALSE,
			(colorMask & RenderState::CmGreen) ? GL_TRUE : GL_FALSE,
			(colorMask & RenderState::CmBlue) ? GL_TRUE : GL_FALSE,
			(colorMask & RenderState::CmAlpha) ? GL_TRUE : GL_FALSE
		));
		m_shadowRenderState.colorMask = colorMask;
	}
#if defined(_DEBUG)
	validate();
#endif
}

void StateCacheOpenGL::setDepthMask(GLboolean depthMask)
{
	if (depthMask != m_shadowRenderState.depthMask)
	{
		T_OGL_SAFE(glDepthMask(depthMask));
		m_shadowRenderState.depthMask = depthMask;
	}
#if defined(_DEBUG)
	validate();
#endif
}

void StateCacheOpenGL::setPermitDepth(bool permitDepth)
{
	if (m_permitDepth != permitDepth)
	{
		m_permitDepth = permitDepth;
		setRenderState(m_shadowRenderState, false);
	}
#if defined(_DEBUG)
	validate();
#endif
}

void StateCacheOpenGL::validate()
{
	GLboolean b = GL_FALSE;
	GLint i = 0;

	if (glIsEnabled(GL_DEPTH_TEST))
	{
		T_ASSERT (m_shadowRenderState.depthTestEnable);

		glGetIntegerv(GL_DEPTH_FUNC, &i);
		T_ASSERT (m_shadowRenderState.depthFunc == i);
	}
	else
	{
		T_ASSERT (!m_shadowRenderState.depthTestEnable);
	}

	glGetBooleanv(GL_DEPTH_WRITEMASK, &b);
	T_ASSERT (m_shadowRenderState.depthMask == b);
}

	}
}
