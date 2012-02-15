#include "Render/OpenGL/Std/Extensions.h"
#include "Render/OpenGL/Std/StateCacheOpenGL.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.StateCacheOpenGL", StateCacheOpenGL, Object)

StateCacheOpenGL::StateCacheOpenGL()
//:	m_arrayBuffer(0)
//,	m_elemArrayBuffer(0)
//,	m_vertexArrayObject(0)
//,	m_program(0)
{
}

void StateCacheOpenGL::setRenderState(const RenderState& renderState)
{
	if (renderState.cullFaceEnable)
	{
		//if (!m_renderState.cullFaceEnable)
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
		//if (cullFace != m_renderState.cullFace)
		{
			T_OGL_SAFE(glCullFace(cullFace));
			m_renderState.cullFace = cullFace;
		}
	}
	else
	{
		//if (m_renderState.cullFaceEnable)
		{
			T_OGL_SAFE(glDisable(GL_CULL_FACE));
			m_renderState.cullFaceEnable = false;
		}
	}

	if (renderState.blendEnable)
	{
		//if (!m_renderState.blendEnable)
		{
			T_OGL_SAFE(glEnable(GL_BLEND));
			m_renderState.blendEnable = true;
		}
		//if (renderState.blendFuncSrc != m_renderState.blendFuncSrc || renderState.blendFuncDest != m_renderState.blendFuncDest)
		{
			T_OGL_SAFE(glBlendFunc(renderState.blendFuncSrc, renderState.blendFuncDest));
			m_renderState.blendFuncSrc = renderState.blendFuncSrc;
			m_renderState.blendFuncDest = renderState.blendFuncDest;
		}
		//if (renderState.blendEquation != m_renderState.blendEquation)
		{
			T_OGL_SAFE(glBlendEquationEXT(renderState.blendEquation));
			m_renderState.blendEquation = renderState.blendEquation;
		}
	}
	else
	{
		//if (m_renderState.blendEnable)
		{
			T_OGL_SAFE(glDisable(GL_BLEND));
			m_renderState.blendEnable = false;
		}
	}

	if (renderState.depthTestEnable && m_permitDepth)
	{
		//if (!m_renderState.depthTestEnable)
		{
			T_OGL_SAFE(glEnable(GL_DEPTH_TEST));
			m_renderState.depthTestEnable = true;
		}
		//if (renderState.depthFunc != m_renderState.depthFunc)
		{
			T_OGL_SAFE(glDepthFunc(renderState.depthFunc));
			m_renderState.depthFunc = renderState.depthFunc;
		}
	}
	else
	{
		//if (m_renderState.depthTestEnable)
		{
			T_OGL_SAFE(glDisable(GL_DEPTH_TEST));
			m_renderState.depthTestEnable = false;
		}
	}

	//if (renderState.colorMask != m_renderState.colorMask)
	{
		T_OGL_SAFE(glColorMask(
			(renderState.colorMask & RenderState::CmRed) ? GL_TRUE : GL_FALSE,
			(renderState.colorMask & RenderState::CmGreen) ? GL_TRUE : GL_FALSE,
			(renderState.colorMask & RenderState::CmBlue) ? GL_TRUE : GL_FALSE,
			(renderState.colorMask & RenderState::CmAlpha) ? GL_TRUE : GL_FALSE
		));
		m_renderState.colorMask = renderState.colorMask;
	}

	//if (renderState.depthMask != m_renderState.depthMask)
	{
		T_OGL_SAFE(glDepthMask(renderState.depthMask));
		m_renderState.depthMask = renderState.depthMask;
	}

	if (renderState.stencilTestEnable && m_permitDepth)
	{
		//if (!m_renderState.stencilTestEnable)
		{
			T_OGL_SAFE(glEnable(GL_STENCIL_TEST));
			m_renderState.stencilTestEnable = true;
		}
		//if (
		//	renderState.stencilFunc != m_renderState.stencilFunc ||
		//	renderState.stencilRef != m_renderState.stencilRef
		//)
		{
			T_OGL_SAFE(glStencilFunc(renderState.stencilFunc, renderState.stencilRef, ~0UL));
			m_renderState.stencilFunc = renderState.stencilFunc;
			m_renderState.stencilRef = renderState.stencilRef;
		}
	}
	else
	{
		//if (m_renderState.stencilTestEnable)
		{
			T_OGL_SAFE(glDisable(GL_STENCIL_TEST));
			m_renderState.stencilTestEnable = false;
		}
	}
}

void StateCacheOpenGL::setColorMask(uint32_t colorMask)
{
	//if (colorMask != m_renderState.colorMask)
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

void StateCacheOpenGL::setDepthMask(GLboolean depthMask)
{
	//if (depthMask != m_renderState.depthMask)
	{
		T_OGL_SAFE(glDepthMask(depthMask));
		m_renderState.depthMask = depthMask;
	}
}

void StateCacheOpenGL::setPermitDepth(bool permitDepth)
{
	m_permitDepth = permitDepth;
	setRenderState(m_renderState);
}

//void StateCacheOpenGL::setArrayBuffer(GLint arrayBuffer)
//{
//	if (m_arrayBuffer != arrayBuffer)
//	{
//		T_OGL_SAFE(glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer));
//		m_arrayBuffer = arrayBuffer;
//	}
//}
//
//void StateCacheOpenGL::setElementArrayBuffer(GLint elemArrayBuffer)
//{
//	if (m_elemArrayBuffer != elemArrayBuffer)
//	{
//		T_OGL_SAFE(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elemArrayBuffer));
//		m_elemArrayBuffer = elemArrayBuffer;
//	}
//}
//
//void StateCacheOpenGL::setVertexArrayObject(GLint vertexArrayObject)
//{
//#if defined(GL_OES_vertex_array_object)
//	if (m_vertexArrayObject != vertexArrayObject)
//	{
//		T_OGL_SAFE(glBindVertexArrayOES(vertexArrayObject));
//		m_vertexArrayObject = vertexArrayObject;
//	}
//#else
//	T_ASSERT (vertexArrayObject != 0);
//#endif
//}
//
//void StateCacheOpenGL::setProgram(GLuint program)
//{
//	if (m_program != program)
//	{
//		T_OGL_SAFE(glUseProgram(program));
//		m_program = program;
//	}
//}

	}
}
