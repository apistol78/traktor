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

namespace traktor
{
	namespace render
	{

/*! \ingroup Render */
//@{

/*! Struct buffer element declaration.
 *
 * This class describe the format of each element of a struct
 * in a struct buffer.
 */
class T_DLLCLASS StructElement
{
public:
	StructElement();

	StructElement(DataType type, uint32_t offset);

	uint32_t getSize() const;

	DataType getDataType() const { return m_type; }

	uint32_t getOffset() const { return m_offset; }

private:
	DataType m_type;
	uint32_t m_offset;
};

uint32_t T_DLLCLASS getStructSize(const AlignedVector< StructElement >& structElements);

uint32_t T_DLLCLASS getStructSize(const StructElement* structElements, uint32_t count);

//@}

	}
}
