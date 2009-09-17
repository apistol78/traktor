#include "Render/OpenGL/IContext.h"
#include "Render/OpenGL/Std/Extensions.h"
#include "Render/OpenGL/Std/VertexBufferVBO.h"
#include "Render/VertexElement.h"
#include "Core/Log/Log.h"

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

VertexBufferVBO::VertexBufferVBO(IContext* context, const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic)
:	VertexBufferOpenGL(bufferSize)
,	m_context(context)
,	m_locked(false)
{
	m_vertexStride = getVertexSize(vertexElements);
	T_ASSERT (m_vertexStride > 0);

	T_OGL_SAFE(glGenBuffersARB(1, &m_name));
	T_OGL_SAFE(glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_name));
	T_OGL_SAFE(glBufferDataARB(GL_ARRAY_BUFFER_ARB, bufferSize, 0, dynamic ? GL_DYNAMIC_DRAW_ARB : GL_STATIC_DRAW_ARB));

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

#if defined(GL_HALF_FLOAT_ARB)
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
	T_ASSERT_M (!m_locked, L"Vertex buffer locked");
	if (m_name)
	{
		if (m_context)
			m_context->deleteResource(new DeleteBufferCallback(m_name));
		m_name = 0;
	}
}

void* VertexBufferVBO::lock()
{
	T_ASSERT_M (!m_locked, L"Vertex buffer already locked");

	T_CONTEXT_SCOPE(m_context);
	T_OGL_SAFE(glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_name));

	void* ptr = glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
	if (!ptr)
		return 0;

	m_locked = true;
	return ptr;
}

void* VertexBufferVBO::lock(uint32_t vertexOffset, uint32_t vertexCount)
{
	T_ASSERT_M (!m_locked, L"Vertex buffer already locked");

	T_CONTEXT_SCOPE(m_context);
	T_OGL_SAFE(glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_name));

	uint8_t* ptr = static_cast< uint8_t* >(glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB));
	if (!ptr)
		return 0;

	m_locked = true;
	return ptr + vertexOffset * m_vertexStride;
}

void VertexBufferVBO::unlock()
{
	T_CONTEXT_SCOPE(m_context);
	T_OGL_SAFE(glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_name));
	T_OGL_SAFE(glUnmapBufferARB(GL_ARRAY_BUFFER_ARB));

	m_locked = false;
}

void VertexBufferVBO::activate(const GLint* attributeLocs)
{
	T_ASSERT_M (!m_locked, L"Vertex buffer locked");

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
