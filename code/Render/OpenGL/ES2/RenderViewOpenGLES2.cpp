#include "Render/OpenGL/ES2/RenderViewOpenGLES2.h"
#include "Render/OpenGL/ES2/RenderSystemOpenGLES2.h"
#include "Render/OpenGL/ES2/VertexBufferOpenGLES2.h"
#include "Render/OpenGL/ES2/IndexBufferOpenGLES2.h"
#include "Render/OpenGL/ES2/ProgramOpenGLES2.h"
#include "Render/OpenGL/ES2/RenderTargetSetOpenGLES2.h"
#include "Render/OpenGL/ES2/RenderTargetOpenGLES2.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderViewOpenGLES2", RenderViewOpenGLES2, IRenderView)


RenderViewOpenGLES2::RenderViewOpenGLES2()
:	m_currentDirty(true)
{
}

RenderViewOpenGLES2::~RenderViewOpenGLES2()
{
}

void RenderViewOpenGLES2::close()
{
}

void RenderViewOpenGLES2::resize(int32_t width, int32_t height)
{
}

void RenderViewOpenGLES2::setViewport(const Viewport& viewport)
{
	T_OGL_SAFE(glViewport(
		viewport.left,
		viewport.top,
		viewport.width,
		viewport.height
	));

	//T_OGL_SAFE(glDepthRange(
	//	viewport.nearZ,
	//	viewport.farZ
	//));
}

Viewport RenderViewOpenGLES2::getViewport()
{
	GLint ext[4];
	T_OGL_SAFE(glGetIntegerv(GL_VIEWPORT, ext));

	GLfloat range[2];
	T_OGL_SAFE(glGetFloatv(GL_DEPTH_RANGE, range));

	Viewport viewport;
	viewport.left = ext[0];
	viewport.top = ext[1];
	viewport.width = ext[2];
	viewport.height = ext[3];
	viewport.nearZ = range[0];
	viewport.farZ = range[1];

	return viewport;
}

bool RenderViewOpenGLES2::begin()
{
	//T_OGL_SAFE(glPushAttrib(GL_VIEWPORT_BIT | GL_DEPTH_BUFFER_BIT));
	T_OGL_SAFE(glEnable(GL_DEPTH_TEST));
	T_OGL_SAFE(glDepthFunc(GL_LEQUAL));

	m_currentDirty = true;
	return true;
}

bool RenderViewOpenGLES2::begin(RenderTargetSet* renderTargetSet, int renderTarget, bool keepDepthStencil)
{
	//T_OGL_SAFE(glPushAttrib(GL_VIEWPORT_BIT | GL_DEPTH_BUFFER_BIT));

	RenderTargetSetOpenGLES2* rts = checked_type_cast< RenderTargetSetOpenGLES2* >(renderTargetSet);
	RenderTargetOpenGLES2* rt = checked_type_cast< RenderTargetOpenGLES2* >(rts->getColorTexture(renderTarget));
	
	rt->bind();
	rt->enter();

	m_renderTargetStack.push(rt);

	rts->setContentValid(true);
	m_currentDirty = true;

	return true;
}

void RenderViewOpenGLES2::clear(uint32_t clearMask, const float color[4], float depth, int32_t stencil)
{
	const GLuint c_clearMask[] =
	{
		0,
		GL_COLOR_BUFFER_BIT,
		GL_DEPTH_BUFFER_BIT,
		GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
		GL_STENCIL_BUFFER_BIT,
		GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
		GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
		GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT
	};

	if (clearMask & CfColor)
	{
		float r = color[0];
		float g = color[1];
		float b = color[2];
		float a = color[3];
		T_OGL_SAFE(glClearColor(r, g, b, a));
		T_OGL_SAFE(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
	}

	if (clearMask & CfDepth)
	{
		//T_OGL_SAFE(glClearDepth(depth));
		T_OGL_SAFE(glDepthMask(GL_TRUE));
	}

	if (clearMask & CfStencil)
		T_OGL_SAFE(glClearStencil(stencil));

	T_OGL_SAFE(glClear(c_clearMask[clearMask]));
}

void RenderViewOpenGLES2::setVertexBuffer(VertexBuffer* vertexBuffer)
{
	m_currentVertexBuffer = checked_type_cast< VertexBufferOpenGLES2* >(vertexBuffer);
	m_currentDirty = true;
}

void RenderViewOpenGLES2::setIndexBuffer(IndexBuffer* indexBuffer)
{
	m_currentIndexBuffer = checked_type_cast< IndexBufferOpenGLES2* >(indexBuffer);
	m_currentDirty = true;
}

void RenderViewOpenGLES2::setProgram(IProgram* program)
{
	m_currentProgram = checked_type_cast< ProgramOpenGLES2 * >(program);
	m_currentDirty = true;
}

void RenderViewOpenGLES2::draw(const Primitives& primitives)
{
	if (m_currentDirty)
	{
		if (!m_currentProgram || !m_currentVertexBuffer)
			return;

		if (!m_currentProgram->activate())
			return;

		m_currentVertexBuffer->activate(
			m_currentProgram->getAttributeLocs()
		);

		m_currentDirty = false;
	}

	GLenum primitiveType;
	GLuint vertexCount;

	switch (primitives.type)
	{
	case PtPoints:
		primitiveType = GL_POINTS;
		vertexCount = primitives.count;
		break;

	case PtLineStrip:
		T_ASSERT_M (0, L"PtLineStrip unsupported");
		break;

	case PtLines:
		primitiveType = GL_LINES;
		vertexCount = primitives.count * 2;
		break;

	case PtTriangleStrip:
		primitiveType = GL_TRIANGLE_STRIP;
		vertexCount = primitives.count + 2;
		break;

	case PtTriangles:
		primitiveType = GL_TRIANGLES;
		vertexCount = primitives.count * 3;
		break;
		
	default:
		T_ASSERT (0);
	}

	if (primitives.indexed)
	{
		T_ASSERT_M (m_currentIndexBuffer, L"No index buffer");

		GLenum indexType;
		GLint offsetMultiplier;

		switch (m_currentIndexBuffer->getIndexType())
		{
		case ItUInt16:
			indexType = GL_UNSIGNED_SHORT;
			offsetMultiplier = 2;
			break;

		case ItUInt32:
			indexType = GL_UNSIGNED_INT;
			offsetMultiplier = 4;
			break;
		}

		m_currentIndexBuffer->bind();

		const GLubyte* indices = static_cast< const GLubyte* >(m_currentIndexBuffer->getIndexData()) + primitives.offset * offsetMultiplier;

		T_OGL_SAFE(glDrawElements(
			primitiveType,
			vertexCount,
			indexType,
			indices
		));
	}
	else
	{
		T_OGL_SAFE(glDrawArrays(
			primitiveType,
			primitives.offset,
			vertexCount
		));
	}
}

void RenderViewOpenGLES2::end()
{
	if (!m_renderTargetStack.empty())
	{
		m_renderTargetStack.pop();

		if (!m_renderTargetStack.empty())
			m_renderTargetStack.top()->bind();
		else
			T_OGL_SAFE(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	}

	//T_OGL_SAFE(glPopAttrib());
}

void RenderViewOpenGLES2::present()
{
}

	}
}
