#ifndef traktor_world_OccluderMesh_H
#define traktor_world_OccluderMesh_H

#include "Core/Object.h"
#include "Core/Math/Vector4.h"
#include "Core/Misc/AutoPtr.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

/*! \brief
 * \ingroup World
 */
class T_DLLCLASS OccluderMesh : public Object
{
	T_RTTI_CLASS;

public:
	OccluderMesh();

	OccluderMesh(uint32_t vertexCount, uint32_t indexCount);

	uint32_t getVertexCount() const { return m_vertexCount; }

	uint32_t getIndexCount() const { return m_indexCount; }

	const float* getVertices() const { return m_vertices.c_ptr(); }

	float* getVertices() { return m_vertices.ptr(); }

	const uint16_t* getIndices() const { return m_indices.c_ptr(); }

	uint16_t* getIndices() { return m_indices.ptr(); }

private:
	friend class OccluderMeshReader;
	friend class OccluderMeshWriter;

	uint32_t m_vertexCount;
	uint32_t m_indexCount;
	AutoArrayPtr< float, AllocFreeAlign > m_vertices;
	AutoArrayPtr< uint16_t, AllocFreeAlign > m_indices;
};

	}
}

#endif	// traktor_world_OccluderMesh_H
