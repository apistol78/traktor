/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include "Render/VertexElement.h"

namespace traktor
{
	namespace render
	{

VertexElement::VertexElement()
:	m_usage(DuPosition)
,	m_type(DtFloat4)
,	m_offset(0)
,	m_index(0)
{
}

VertexElement::VertexElement(DataUsage usage, DataType type, uint32_t offset, uint32_t index)
:	m_usage(usage)
,	m_type(type)
,	m_offset(offset)
,	m_index(index)
{
}

uint32_t VertexElement::getSize() const
{
	uint32_t c_typeSize[] =
	{
		1 * sizeof(float),
		2 * sizeof(float),
		3 * sizeof(float),
		4 * sizeof(float),
		4 * sizeof(char),
		4 * sizeof(char),
		2 * sizeof(short),
		4 * sizeof(short),
		2 * sizeof(short),
		4 * sizeof(short),
		2 * sizeof(float) / 2,
		4 * sizeof(float) / 2
	};
	return c_typeSize[m_type];
}

uint32_t getVertexSize(const std::vector< VertexElement >& vertexElements)
{
	return !vertexElements.empty() ? getVertexSize(&vertexElements[0], int(vertexElements.size())) : 0;
}

uint32_t getVertexSize(const VertexElement* vertexElements, uint32_t count)
{
	uint32_t size = 0;
	for (uint32_t i = 0; i < count; ++i)
	{
		uint32_t next = vertexElements[i].getOffset() + vertexElements[i].getSize();
		size = std::max< uint32_t >(size, next);
	}
	return size;
}

std::vector< VertexElement >::const_iterator findVertexElement(
	const std::vector< VertexElement >& vertexElements,
	DataUsage usage,
	uint32_t index
)
{
	for (std::vector< VertexElement >::const_iterator i = vertexElements.begin(); i != vertexElements.end(); ++i)
	{
		if (i->getDataUsage() == usage && i->getIndex() == index)
			return i;
	}
	return vertexElements.end();
}

	}
}
