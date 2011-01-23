#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/IContext.h"
#include "Render/OpenGL/ES2/VertexBufferOpenGLES2.h"
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
		T_OGL_SAFE(glDeleteBuffers(1, &m_bufferName));
		delete this;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferOpenGLES2", VertexBufferOpenGLES2, VertexBufferOpenGL)

VertexBufferOpenGLES2::VertexBufferOpenGLES2(IContext* context, const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic)
:	VertexBufferOpenGL(bufferSize)
,	m_context(context)
,	m_dynamic(dynamic)
{
	m_vertexStride = getVertexSize(vertexElements);
	T_ASSERT (m_vertexStride > 0);

	T_OGL_SAFE(glGenBuffers(1, &m_name));
	T_OGL_SAFE(glBindBuffer(GL_ARRAY_BUFFER, m_name));
	T_OGL_SAFE(glBufferData(GL_ARRAY_BUFFER, bufferSize, 0, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW));

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

#if defined(GL_OES_texture_half_float)
		case DtHalf2:
			m_attributeDesc[usageIndex].size = 2;
			m_attributeDesc[usageIndex].type = GL_HALF_FLOAT_OES;
			m_attributeDesc[usageIndex].normalized = GL_TRUE;
			break;

		case DtHalf4:
			m_attributeDesc[usageIndex].size = 4;
			m_attributeDesc[usageIndex].type = GL_HALF_FLOAT_OES;
			m_attributeDesc[usageIndex].normalized = GL_TRUE;
			break;
#endif

		default:
			log::warning << L"Unsupport vertex format" << Endl;
		}

		m_attributeDesc[usageIndex].offset = vertexElements[i].getOffset();
	}
}

VertexBufferOpenGLES2::~VertexBufferOpenGLES2()
{
	destroy();
}

void VertexBufferOpenGLES2::destroy()
{
	if (m_name)
	{
		if (m_context)
			m_context->deleteResource(new DeleteBufferCallback(m_name));
		m_name = 0;
	}
}

void* VertexBufferOpenGLES2::lock()
{
	if (m_buffer.ptr())
		return 0;

	int32_t bufferSize = getBufferSize();
	m_buffer.reset((uint8_t*)Alloc::acquireAlign(bufferSize, 16, "VB"));

	return m_buffer.ptr();
}

void* VertexBufferOpenGLES2::lock(uint32_t vertexOffset, uint32_t vertexCount)
{
	T_FATAL_ERROR;
	return 0;
}

void VertexBufferOpenGLES2::unlock()
{
	if (!m_buffer.ptr())
		return;

	int32_t bufferSize = getBufferSize();
	T_OGL_SAFE(glBindBuffer(GL_ARRAY_BUFFER, m_name));
	T_OGL_SAFE(glBufferData(GL_ARRAY_BUFFER, bufferSize, m_buffer.ptr(), m_dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW));
	//m_buffer.release();

	setContentValid(true);
}

void VertexBufferOpenGLES2::activate(const GLint* attributeLocs)
{
	T_OGL_SAFE(glBindBuffer(GL_ARRAY_BUFFER, m_name));
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
}

	}
}
