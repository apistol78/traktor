#pragma once

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Render/VertexElement.h"

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

class Mesh;

/*! Render mesh factory.
 * \ingroup Render
 */
class T_DLLCLASS MeshFactory : public Object
{
	T_RTTI_CLASS;

public:
	virtual Ref< Mesh > createMesh(
		const AlignedVector< VertexElement >& vertexElements,
		uint32_t vertexBufferSize,
		IndexType indexType,
		uint32_t indexBufferSize
	) const = 0;
};

	}
}

