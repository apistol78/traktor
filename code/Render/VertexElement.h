#pragma once

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

/*! \ingroup Render */
//@{

/*! Vertex buffer element declaration.
 *
 * This class describe the format of each element of a vertex
 * in a vertex buffer.
 */
class T_DLLCLASS VertexElement
{
public:
	VertexElement() = default;

	explicit VertexElement(DataUsage usage, DataType type, uint32_t offset, uint32_t index = 0);

	uint32_t getSize() const;

	DataUsage getDataUsage() const { return m_usage; }

	DataType getDataType() const { return m_type; }

	uint32_t getOffset() const { return m_offset; }

	uint32_t getIndex() const { return m_index; }

private:
	DataUsage m_usage = DataUsage::Position;
	DataType m_type = DtFloat4;
	uint32_t m_offset = 0;
	uint32_t m_index = 0;
};

uint32_t T_DLLCLASS getVertexSize(const AlignedVector< VertexElement >& vertexElements);

uint32_t T_DLLCLASS getVertexSize(const VertexElement* vertexElements, uint32_t count);

AlignedVector< VertexElement >::const_iterator T_DLLCLASS findVertexElement(
	const AlignedVector< VertexElement >& vertexElements,
	DataUsage usage,
	uint32_t index
);

//@}

}
