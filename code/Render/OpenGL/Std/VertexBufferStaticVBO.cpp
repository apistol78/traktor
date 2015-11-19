#include <cstring>
#include "Core/Log/Log.h"
#include "Render/VertexElement.h"
#include "Render/OpenGL/Std/ContextOpenGL.h"
#include "Render/OpenGL/Std/VertexBufferStaticVBO.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct DeleteBufferCallback : public ContextOpenGL::IDeleteCallback
{
	GLuint m_buffer;

	DeleteBufferCallback(GLuint buffer)
	:	m_buffer(buffer)
	{
	}

	virtual void deleteResource()
	{
		T_OGL_SAFE(glDeleteBuffers(1, &m_buffer));
		delete this;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferStaticVBO", VertexBufferStaticVBO, VertexBufferOpenGL)

VertexBufferStaticVBO::VertexBufferStaticVBO(ContextOpenGL* resourceContext, const std::vector< VertexElement >& vertexElements, uint32_t bufferSize)
:	VertexBufferOpenGL(bufferSize)
,	m_resourceContext(resourceContext)
,	m_array(0)
,	m_buffer(0)
,	m_attributeLocs(0)
,	m_lock(0)
{
	m_vertexStride = getVertexSize(vertexElements);
	T_ASSERT (m_vertexStride > 0);

	T_OGL_SAFE(glGenBuffers(1, &m_buffer));
	T_OGL_SAFE(glBindBuffer(GL_ARRAY_BUFFER, m_buffer));
	T_OGL_SAFE(glBufferData(GL_ARRAY_BUFFER, bufferSize, 0, GL_STATIC_DRAW));

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

		case DtHalf2:
			m_attributeDesc[usageIndex].size = 2;
			m_attributeDesc[usageIndex].type = GL_HALF_FLOAT;
			m_attributeDesc[usageIndex].normalized = GL_TRUE;
			break;

		case DtHalf4:
			m_attributeDesc[usageIndex].size = 4;
			m_attributeDesc[usageIndex].type = GL_HALF_FLOAT;
			m_attributeDesc[usageIndex].normalized = GL_TRUE;
			break;

		default:
			log::error << L"Unsupport vertex format" << Endl;
		}

		m_attributeDesc[usageIndex].offset = vertexElements[i].getOffset();
	}
}

VertexBufferStaticVBO::~VertexBufferStaticVBO()
{
	destroy();
}

void VertexBufferStaticVBO::destroy()
{
	if (m_buffer)
	{
		if (m_resourceContext)
			m_resourceContext->deleteResource(new DeleteBufferCallback(m_buffer));
		m_buffer = 0;
	}
}

void* VertexBufferStaticVBO::lock()
{
	T_ASSERT_M(!m_lock, L"Vertex buffer already locked");
	T_ANONYMOUS_VAR(ContextOpenGL::Scope)(m_resourceContext);
	
	T_OGL_SAFE(glBindBuffer(GL_ARRAY_BUFFER, m_buffer));
	m_lock = static_cast< uint8_t* >(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
	
	return m_lock;
}

void* VertexBufferStaticVBO::lock(uint32_t vertexOffset, uint32_t vertexCount)
{
	T_ASSERT_M(!m_lock, L"Vertex buffer already locked");
	T_ANONYMOUS_VAR(ContextOpenGL::Scope)(m_resourceContext);
	
	T_OGL_SAFE(glBindBuffer(GL_ARRAY_BUFFER, m_buffer));
	m_lock = static_cast< uint8_t* >(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
	
	return m_lock ? (m_lock + vertexOffset * m_vertexStride) : 0;
}

void VertexBufferStaticVBO::unlock()
{
	T_ASSERT_M(m_lock, L"Vertex buffer not locked");
	T_ANONYMOUS_VAR(ContextOpenGL::Scope)(m_resourceContext);
	
	T_OGL_SAFE(glBindBuffer(GL_ARRAY_BUFFER, m_buffer));
	T_OGL_SAFE(glUnmapBuffer(GL_ARRAY_BUFFER));
	
	m_lock = 0;
	m_attributeLocs = 0;
	
	setContentValid(true);
}

void VertexBufferStaticVBO::activate(const GLint* attributeLocs)
{
	T_ASSERT_M(!m_lock, L"Vertex buffer still locked");

	if (!m_array || attributeLocs != m_attributeLocs)
	{
		if (!m_array)
			T_OGL_SAFE(glGenVertexArrays(1, &m_array));

		T_OGL_SAFE(glBindVertexArray(m_array));
		T_OGL_SAFE(glBindBuffer(GL_ARRAY_BUFFER, m_buffer));

		for (int i = 0; i < T_OGL_MAX_USAGE_INDEX; ++i)
		{
			if (attributeLocs[i] == -1 || m_attributeDesc[i].size == 0)
				continue;

			T_OGL_SAFE(glEnableVertexAttribArray(attributeLocs[i]));
			T_OGL_SAFE(glVertexAttribPointer(
				attributeLocs[i],
				m_attributeDesc[i].size,
				m_attributeDesc[i].type,
				m_attributeDesc[i].normalized,
				m_vertexStride,
				(GLvoid*)m_attributeDesc[i].offset
			));
		}

		m_attributeLocs = attributeLocs;
	}
	else
	{
		T_OGL_SAFE(glBindVertexArray(m_array));
	}
}

	}
}
