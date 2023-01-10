/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Render/VertexElement.h"

namespace traktor::render
{

VertexElement::VertexElement(DataUsage usage, DataType type, uint32_t offset, uint32_t index)
:	m_usage(usage)
,	m_type(type)
,	m_offset(offset)
,	m_index(index)
{
}

uint32_t VertexElement::getSize() const
{
	const uint32_t c_typeSize[] =
	{
		1 * sizeof(float),
		2 * sizeof(float),
		3 * sizeof(float),
		4 * sizeof(float),
		4 * sizeof(int8_t),
		4 * sizeof(int8_t),
		2 * sizeof(int16_t),
		4 * sizeof(int16_t),
		2 * sizeof(int16_t),
		4 * sizeof(int16_t),
		2 * sizeof(float) / 2,
		4 * sizeof(float) / 2,
		1 * sizeof(int32_t),
		2 * sizeof(int32_t),
		3 * sizeof(int32_t),
		4 * sizeof(int32_t)
	};
	return c_typeSize[m_type];
}

uint32_t getVertexSize(const AlignedVector< VertexElement >& vertexElements)
{
	return !vertexElements.empty() ? getVertexSize(&vertexElements[0], uint32_t(vertexElements.size())) : 0;
}

uint32_t getVertexSize(const VertexElement* vertexElements, uint32_t count)
{
	uint32_t size = 0;
	for (uint32_t i = 0; i < count; ++i)
	{
		const uint32_t next = vertexElements[i].getOffset() + vertexElements[i].getSize();
		size = std::max< uint32_t >(size, next);
	}
	return size;
}

AlignedVector< VertexElement >::const_iterator findVertexElement(
	const AlignedVector< VertexElement >& vertexElements,
	DataUsage usage,
	uint32_t index
)
{
	for (auto i = vertexElements.begin(); i != vertexElements.end(); ++i)
	{
		if (i->getDataUsage() == usage && i->getIndex() == index)
			return i;
	}
	return vertexElements.end();
}

}
