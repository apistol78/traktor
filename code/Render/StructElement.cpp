#include <algorithm>
#include "Render/StructElement.h"

namespace traktor
{
	namespace render
	{

StructElement::StructElement(DataType type, uint32_t offset)
:	m_type(type)
,	m_offset(offset)
{
}

uint32_t StructElement::getSize() const
{
	uint32_t c_typeSize[] =
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

uint32_t getStructSize(const AlignedVector< StructElement >& structElements)
{
	return !structElements.empty() ? getStructSize(&structElements[0], uint32_t(structElements.size())) : 0;
}

uint32_t getStructSize(const StructElement* structElements, uint32_t count)
{
	uint32_t size = 0;

	// Calculate total size of all elements.
	for (uint32_t i = 0; i < count; ++i)
	{
		uint32_t next = structElements[i].getOffset() + structElements[i].getSize();
		size = std::max< uint32_t >(size, next);
	}

	// Ensure structs are aligned on 16 byte boundary, assuming GLSL layout.
	return alignUp(size, 16);
}

	}
}
