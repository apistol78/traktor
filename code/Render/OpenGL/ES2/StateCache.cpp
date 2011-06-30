#include "Render/OpenGL/ES2/StateCache.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.StateCache", StateCache, Object)

StateCache::StateCache()
:	m_arrayBuffer(0)
,	m_elemArrayBuffer(0)
,	m_vertexArrayObject(0)
,	m_program(0)
{
}

void StateCache::setRenderState(const RenderState& renderState)
{
	if (renderState.cullFaceEnable)
	{
		if (!m_renderState.cullFaceEnable)
		{
			T_OGL_SAFE(glEnable(GL_CULL_FACE));
			m_renderState.cullFaceEnable = true;
		}
		GLuint cullFace = renderState.cullFace;
		if (true)
		{
			if (cullFace == GL_FRONT)
				cullFace = GL_BACK;
			else
				cullFace = GL_FRONT;
		}
		if (cullFace != m_renderState.cullFace)
		{
			T_OGL_SAFE(glCullFace(cullFace));
			m_renderState.cullFace = cullFace;
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
}

void StateCache::setColorMask(uint32_t colorMask)
{
	if (colorMask != m_renderState.colorMask)
	{
		T_OGL_SAFE(glColorMask(
			(colorMask & RenderState::CmRed) ? GL_TRUE : GL_FALSE,
			(colorMask & RenderState::CmGreen) ? GL_TRUE : GL_FALSE,
			(colorMask & RenderState::CmBlue) ? GL_TRUE : GL_FALSE,
			(colorMask & RenderState::CmAlpha) ? GL_TRUE : GL_FALSE
		));
		m_renderState.colorMask = colorMask;
	}
}

void StateCache::setDepthMask(GLboolean depthMask)
{
	if (depthMask != m_renderState.depthMask)
	{
		T_OGL_SAFE(glDepthMask(depthMask));
		m_renderState.depthMask = depthMask;
	}
}

void StateCache::setArrayBuffer(GLint arrayBuffer)
{
	if (m_arrayBuffer != arrayBuffer)
	{
		T_OGL_SAFE(glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer));
		m_arrayBuffer = arrayBuffer;
	}
}

void StateCache::setElementArrayBuffer(GLint elemArrayBuffer)
{
	if (m_elemArrayBuffer != elemArrayBuffer)
	{
		T_OGL_SAFE(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elemArrayBuffer));
		m_elemArrayBuffer = elemArrayBuffer;
	}
}

void StateCache::setVertexArrayObject(GLint vertexArrayObject)
{
#if defined(GL_OES_vertex_array_object)
	if (m_vertexArrayObject != vertexArrayObject)
	{
		T_OGL_SAFE(glBindVertexArrayOES(vertexArrayObject));
		m_vertexArrayObject = vertexArrayObject;
	}
#else
	T_ASSERT (vertexArrayObject != 0);
#endif
}

void StateCache::setProgram(GLuint program)
{
	if (m_program != program)
	{
		T_OGL_SAFE(glUseProgram(program));
		m_program = program;
	}
}

	}
}
