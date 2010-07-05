#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Memory/Alloc.h"
#include "Render/VertexElement.h"
#include "Render/OpenGL/IContext.h"
#include "Render/OpenGL/Std/Extensions.h"
#include "Render/OpenGL/Std/VertexBufferVBO.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct DeleteBufferCallback : public IContext::IDeleteCallback
{
	GLuint m_bufferName;

	DeleteBufferCallback(GLuint bufferName)
	:	m_bufferName(bufferName)
	{
	}

	virtual void deleteResource()
	{
		T_OGL_SAFE(glDeleteBuffersARB(1, &m_bufferName));
		delete this;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferVBO", VertexBufferVBO, VertexBufferOpenGL)

VertexBufferVBO::VertexBufferVBO(IContext* resourceContext, const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic)
:	VertexBufferOpenGL(bufferSize)
,	m_resourceContext(resourceContext)
,	m_lock(0)
,	m_dynamic(dynamic)
{
	m_vertexStride = getVertexSize(vertexElements);
	T_ASSERT (m_vertexStride > 0);

	T_OGL_SAFE(glGenBuffersARB(1, &m_name));
	T_OGL_SAFE(glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_name));
	T_OGL_SAFE(glBufferDataARB(GL_ARRAY_BUFFER_ARB, bufferSize, 0, m_dynamic ? GL_DYNAMIC_DRAW_ARB : GL_STATIC_DRAW_ARB));

	std::memset(m_attributeDesc, 0, sizeof(m_attributeDesc));

	for (size_t i = 0; i < vertexElements.size(); ++i)
	{
		if (vertexElements[i].getIndex() >= 4)
		{
			log::warning << L"Index out of bounds on vertex element " << uint32_t(i) << Endl;
			continue;
		}

		int usageIndex = T_OGL_USAGE_INDEX(vertexElements[i].getDataUsage(), vertexElements[i].getIndex());
		switch (vertexElements[i].getDataType())
		{
		case DtFloat1:
			m_attributeDesc[usageIndex].size = 1;
			m_attributeDesc[usageIndex].type = GL_FLOAT;
			m_attributeDesc[usageIndex].normalized = GL_FALSE;
			break;

		case DtFloat2:
			m_attributeDesc[usageIndex].size = 2;
			m_attributeDesc[usageIndex].type = GL_FLOAT;
			m_attributeDesc[usageIndex].normalized = GL_FALSE;
			break;

		case DtFloat3:
			m_attributeDesc[usageIndex].size = 3;
			m_attributeDesc[usageIndex].type = GL_FLOAT;
			m_attributeDesc[usageIndex].normalized = GL_FALSE;
			break;

		case DtFloat4:
			m_attributeDesc[usageIndex].size = 4;
			m_attributeDesc[usageIndex].type = GL_FLOAT;
			m_attributeDesc[usageIndex].normalized = GL_FALSE;
			break;

		case DtByte4:
			m_attributeDesc[usageIndex].size = 4;
			m_attributeDesc[usageIndex].type = GL_UNSIGNED_BYTE;
			m_attributeDesc[usageIndex].normalized = GL_FALSE;
			break;

		case DtByte4N:
			m_attributeDesc[usageIndex].size = 4;
			m_attributeDesc[usageIndex].type = GL_UNSIGNED_BYTE;
			m_attributeDesc[usageIndex].normalized = GL_TRUE;
			break;

		case DtShort2:
			m_attributeDesc[usageIndex].size = 2;
			m_attributeDesc[usageIndex].type = GL_SHORT;
			m_attributeDesc[usageIndex].normalized = GL_FALSE;
			break;

		case DtShort4:
			m_attributeDesc[usageIndex].size = 4;
			m_attributeDesc[usageIndex].type = GL_SHORT;
			m_attributeDesc[usageIndex].normalized = GL_FALSE;
			break;

		case DtShort2N:
			m_attributeDesc[usageIndex].size = 2;
			m_attributeDesc[usageIndex].type = GL_SHORT;
			m_attributeDesc[usageIndex].normalized = GL_TRUE;
			break;

		case DtShort4N:
			m_attributeDesc[usageIndex].size = 4;
			m_attributeDesc[usageIndex].type = GL_SHORT;
			m_attributeDesc[usageIndex].normalized = GL_TRUE;
			break;

#if defined(GL_ARB_half_float_pixel)
		case DtHalf2:
			m_attributeDesc[usageIndex].size = 2;
			m_attributeDesc[usageIndex].type = GL_HALF_FLOAT_ARB;
			m_attributeDesc[usageIndex].normalized = GL_TRUE;
			break;

		case DtHalf4:
			m_attributeDesc[usageIndex].size = 4;
			m_attributeDesc[usageIndex].type = GL_HALF_FLOAT_ARB;
			m_attributeDesc[usageIndex].normalized = GL_TRUE;
			break;
#endif

		default:
			log::warning << L"Unsupport vertex format" << Endl;
		}

		m_attributeDesc[usageIndex].offset = vertexElements[i].getOffset();
	}
}

VertexBufferVBO::~VertexBufferVBO()
{
	destroy();
}

void VertexBufferVBO::destroy()
{
	if (m_name)
	{
		if (m_resourceContext)
			m_resourceContext->deleteResource(new DeleteBufferCallback(m_name));
		m_name = 0;
	}
}

void* VertexBufferVBO::lock()
{
	T_ASSERT_M(!m_lock, L"Vertex buffer already locked");
	m_lock = (uint8_t*)Alloc::acquireAlign(getBufferSize(), 16);
	return m_lock;
}

void* VertexBufferVBO::lock(uint32_t vertexOffset, uint32_t vertexCount)
{
	T_FATAL_ERROR;
	return 0;
}

void VertexBufferVBO::unlock()
{
	T_ASSERT_M(m_lock, L"Vertex buffer not locked");
	T_ANONYMOUS_VAR(IContext::Scope)(m_resourceContext);

	T_OGL_SAFE(glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_name));
	T_OGL_SAFE(glBufferDataARB(GL_ARRAY_BUFFER_ARB, getBufferSize(), m_lock, m_dynamic ? GL_DYNAMIC_DRAW_ARB : GL_STATIC_DRAW_ARB));

	Alloc::freeAlign(m_lock);
	m_lock = 0;
	
	setContentValid(true);
}

void VertexBufferVBO::activate(const GLint* attributeLocs)
{
	T_ASSERT_M(!m_lock, L"Vertex buffer still locked");

	T_OGL_SAFE(glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_name));

	for (int i = 0; i < T_OGL_MAX_USAGE_INDEX; ++i)
	{
		if (attributeLocs[i] == -1 || m_attributeDesc[i].size == 0)
			continue;

		T_OGL_SAFE(glEnableVertexAttribArrayARB(attributeLocs[i]));
		T_OGL_SAFE(glVertexAttribPointerARB(
			attributeLocs[i],
			m_attributeDesc[i].size,
			m_attributeDesc[i].type,
			m_attributeDesc[i].normalized,
			m_vertexStride,
			(GLvoid*)m_attributeDesc[i].offset
		));
	}
}

	}
}
