#include "Core/Io/Writer.h"
#include "World/SwHiZ/OccluderMesh.h"
#include "World/SwHiZ/OccluderMeshWriter.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.OccluderMeshWriter", OccluderMeshWriter, Object)

bool OccluderMeshWriter::write(IStream* stream, const OccluderMesh* mesh) const
{
	Writer writer(stream);

	writer << uint32_t(1);
	writer << mesh->m_vertexCount;
	writer << mesh->m_indexCount;

	if (mesh->m_vertexCount > 0)
		writer.write(mesh->m_vertices.c_ptr(), mesh->m_vertexCount * 4, sizeof(float));

	if (mesh->m_indexCount > 0)
		writer.write(mesh->m_indices.c_ptr(), mesh->m_indexCount, sizeof(uint16_t));

	return true;
}

	}
}
