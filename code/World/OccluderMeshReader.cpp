#include "Core/Io/Reader.h"
#include "Core/Misc/Endian.h"
#include "World/OccluderMesh.h"
#include "World/OccluderMeshReader.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.OccluderMeshReader", OccluderMeshReader, Object)

Ref< OccluderMesh > OccluderMeshReader::read(IStream* stream) const
{
	Reader reader(stream);

	uint32_t version;
	reader >> version;
	if (version != 1)
		return 0;

	uint32_t vertexCount, indexCount;
	reader >> vertexCount;
	reader >> indexCount;

	Ref< OccluderMesh > mesh = new OccluderMesh(vertexCount, indexCount);

	if (mesh->m_vertexCount > 0)
	{
		reader.read(mesh->m_vertices.ptr(), mesh->m_vertexCount * 4, sizeof(float));

#if !defined(T_LITTLE_ENDIAN)
		for (uint32_t i = 0; i < mesh->m_vertexCount; ++i)
			swap8in32(mesh->m_vertices[i]);
#endif
	}

	if (mesh->m_indexCount > 0)
	{
		reader.read(mesh->m_indices.ptr(), mesh->m_indexCount, sizeof(uint16_t));

#if !defined(T_LITTLE_ENDIAN)
		for (uint32_t i = 0; i < mesh->m_indexCount; ++i)
			swap8in32(mesh->m_indices[i]);
#endif
	}

	return mesh;
}

	}
}
