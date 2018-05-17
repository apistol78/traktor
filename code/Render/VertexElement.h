/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_VertexElement_H
#define traktor_render_VertexElement_H

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

namespace traktor
{
	namespace render
	{

/*! \ingroup Render */
//@{

/*! \brief Vertex buffer element declaration.
 *
 * This class describe the format of each element of a vertex
 * in a vertex buffer.
 */
class T_DLLCLASS VertexElement
{
public:
	VertexElement();

	VertexElement(DataUsage usage, DataType type, uint32_t offset, uint32_t index = 0);

	uint32_t getSize() const;

	DataUsage getDataUsage() const { return m_usage; }
	
	DataType getDataType() const { return m_type; }
	
	uint32_t getOffset() const { return m_offset; }
	
	uint32_t getIndex() const { return m_index; }
	
private:
	DataUsage m_usage;
	DataType m_type;
	unsigned int m_offset;
	unsigned int m_index;
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
}

#endif	// traktor_render_VertexElement_H
