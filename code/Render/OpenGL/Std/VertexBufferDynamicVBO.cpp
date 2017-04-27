/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Math/MathConfig.h"
#include "Render/VertexElement.h"
#include "Render/OpenGL/Std/ContextOpenGL.h"
#include "Render/OpenGL/Std/VertexBufferDynamicVBO.h"

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

#if defined(T_MATH_USE_SSE2) && !defined(_DEBUG)
void copyBuffer(uint8_t* dst, const uint8_t* src, uint32_t size)
{
	uint32_t i = 0;
	for (; i + 128 <= size; i += 128)
	{
		__m128i d0 = _mm_load_si128((__m128i*)&src[i + 0 * 16]);
		__m128i d1 = _mm_load_si128((__m128i*)&src[i + 1 * 16]);
		__m128i d2 = _mm_load_si128((__m128i*)&src[i + 2 * 16]);
		__m128i d3 = _mm_load_si128((__m128i*)&src[i + 3 * 16]);
		__m128i d4 = _mm_load_si128((__m128i*)&src[i + 4 * 16]);
		__m128i d5 = _mm_load_si128((__m128i*)&src[i + 5 * 16]);
		__m128i d6 = _mm_load_si128((__m128i*)&src[i + 6 * 16]);
		__m128i d7 = _mm_load_si128((__m128i*)&src[i + 7 * 16]);
		_mm_stream_si128((__m128i*)&dst[i + 0 * 16], d0);
		_mm_stream_si128((__m128i*)&dst[i + 1 * 16], d1);
		_mm_stream_si128((__m128i*)&dst[i + 2 * 16], d2);
		_mm_stream_si128((__m128i*)&dst[i + 3 * 16], d3);
		_mm_stream_si128((__m128i*)&dst[i + 4 * 16], d4);
		_mm_stream_si128((__m128i*)&dst[i + 5 * 16], d5);
		_mm_stream_si128((__m128i*)&dst[i + 6 * 16], d6);
		_mm_stream_si128((__m128i*)&dst[i + 7 * 16], d7);
	}
	for (; i + 16 <= size; i += 16)
	{
		__m128i d = _mm_load_si128((__m128i*)&src[i]);
		_mm_stream_si128((__m128i *)&dst[i], d);
	}
	for (; i + 4 <= size; i += 4)
	{
		*(uint32_t*)&dst[i] = *(const uint32_t*)&src[i];
	}
	for (; i < size; i++)
	{
		dst[i] = src[i];
	}
}
#else
void copyBuffer(uint8_t* dst, const uint8_t* src, uint32_t size)
{
	std::memcpy(dst, src, size);
}
#endif

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferDynamicVBO", VertexBufferDynamicVBO, VertexBufferOpenGL)

VertexBufferDynamicVBO::VertexBufferDynamicVBO(ContextOpenGL* resourceContext, const std::vector< VertexElement >& vertexElements, uint32_t bufferSize)
:	VertexBufferOpenGL(bufferSize)
,	m_resourceContext(resourceContext)
,	m_array(0)
,	m_buffer(0)
,	m_vertexStride(0)
,	m_attributeLocs(0)
,	m_lock(0)
,	m_mapped(0)
,	m_dirty(false)
{
	m_vertexStride = getVertexSize(vertexElements);
	T_ASSERT (m_vertexStride > 0);

	T_OGL_SAFE(glGenBuffers(1, &m_buffer));
	T_OGL_SAFE(glBindBuffer(GL_ARRAY_BUFFER, m_buffer));

#if !defined(__APPLE__)
	if (GLEW_ARB_buffer_storage && GLEW_VERSION_4_4)
	{
		T_OGL_SAFE(glBufferStorage(
			GL_ARRAY_BUFFER,
			bufferSize,
			0,
			GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT
		));
	}
	else
#endif
	{
		T_OGL_SAFE(glBufferData(
			GL_ARRAY_BUFFER, 
			bufferSize,
			0,
			GL_DYNAMIC_DRAW
		));
	}

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

	m_data.resize(getBufferSize(), 0);
	m_dirty = true;
}

VertexBufferDynamicVBO::~VertexBufferDynamicVBO()
{
	destroy();
}

void VertexBufferDynamicVBO::destroy()
{
	if (m_buffer)
	{
		if (m_resourceContext)
			m_resourceContext->deleteResource(new DeleteBufferCallback(m_buffer));
		m_buffer = 0;
	}

	m_data.resize(0);
}

void* VertexBufferDynamicVBO::lock()
{
	T_ASSERT_M(!m_lock, L"Vertex buffer already locked");

	m_lock = &m_data[0];
	return m_lock;
}

void* VertexBufferDynamicVBO::lock(uint32_t vertexOffset, uint32_t vertexCount)
{
	T_ASSERT_M(!m_lock, L"Vertex buffer already locked");

	m_lock = &m_data[0];
	if (!m_lock)
		return 0;

	return (uint8_t*)m_lock + vertexOffset * m_vertexStride;
}

void VertexBufferDynamicVBO::unlock()
{
	T_ASSERT_M(m_lock, L"Vertex buffer not locked");

	m_lock = 0;
	m_dirty = true;
	m_attributeLocs = 0;

	setContentValid(true);
}

void VertexBufferDynamicVBO::activate(const GLint* attributeLocs)
{
	T_ASSERT_M(!m_lock, L"Vertex buffer still locked");

	if (!m_array || attributeLocs != m_attributeLocs || m_dirty)
	{
		if (!m_array)
			T_OGL_SAFE(glGenVertexArrays(1, &m_array));

		T_OGL_SAFE(glBindVertexArray(m_array));
		T_OGL_SAFE(glBindBuffer(GL_ARRAY_BUFFER, m_buffer));

		if (m_dirty)
		{
#if !defined(__APPLE__)
			bool usePersistent = bool(GLEW_ARB_buffer_storage && GLEW_VERSION_4_4);
			if ( !usePersistent )
				m_mapped = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
			else if (!m_mapped)
				m_mapped = glMapBufferRange(GL_ARRAY_BUFFER, 0, getBufferSize(), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
#else
			m_mapped = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
#endif

			if (!m_mapped)
				return;

			copyBuffer((uint8_t*)m_mapped, (const uint8_t*)&m_data[0], getBufferSize());

#if !defined(__APPLE__)
			if ( !usePersistent )
#endif
			{
				T_OGL_SAFE(glUnmapBuffer(GL_ARRAY_BUFFER));
			}

			m_dirty = false;
		}

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
	else
	{
		T_OGL_SAFE(glBindVertexArray(m_array));
	}
}

	}
}
