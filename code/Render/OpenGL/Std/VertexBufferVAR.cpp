/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cstring>
#include "Core/Log/Log.h"
#include "Render/VertexElement.h"
#include "Render/OpenGL/Std/VertexBufferVAR.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferVAR", VertexBufferVAR, VertexBufferOpenGL)

VertexBufferVAR::VertexBufferVAR(ContextOpenGL* resourceContext, const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic)
:	VertexBufferOpenGL(bufferSize)
,	m_resourceContext(resourceContext)
,	m_dynamic(dynamic)
,	m_data(0)
{
	m_vertexStride = getVertexSize(vertexElements);

	m_data = new GLubyte [bufferSize];
	T_ASSERT (m_data);

	std::memset(m_data, 0, bufferSize);
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

VertexBufferVAR::~VertexBufferVAR()
{
	destroy();
}

void VertexBufferVAR::destroy()
{
	delete[] m_data;
	m_data = 0;
}

void* VertexBufferVAR::lock()
{
	return m_data;
}

void* VertexBufferVAR::lock(uint32_t vertexOffset, uint32_t vertexCount)
{
	return m_data + vertexOffset * m_vertexStride;
}

void VertexBufferVAR::unlock()
{
	setContentValid(true);
}

void VertexBufferVAR::activate(const GLint* attributeLocs)
{
	T_ASSERT (m_data);
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
			&m_data[m_attributeDesc[i].offset]
		));
	}
}

	}
}
