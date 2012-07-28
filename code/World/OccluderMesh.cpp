#include "World/OccluderMesh.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.OccluderMesh", OccluderMesh, Object)

OccluderMesh::OccluderMesh()
:	m_vertexCount(0)
,	m_indexCount(0)
{
}

OccluderMesh::OccluderMesh(uint32_t vertexCount, uint32_t indexCount)
:	m_vertexCount(vertexCount)
,	m_indexCount(indexCount)
{
	m_vertices.reset((float*)Alloc::acquireAlign(vertexCount * 4 * sizeof(float), 16, "Occluder mesh vertices"));
	m_indices.reset((uint16_t*)Alloc::acquireAlign(indexCount * sizeof(uint16_t), 16, "Occluder mesh indices"));
}

	}
}
