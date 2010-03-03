#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStl.h"
#include "Mesh/Stream/StreamMesh.h"
#include "Mesh/Stream/StreamMeshResource.h"
#include "Render/Mesh/MeshReader.h"
#include "Resource/IResourceManager.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.StreamMeshResource", 0, StreamMeshResource, IMeshResource)

Ref< IMesh > StreamMeshResource::createMesh(
	IStream* dataStream,
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	render::MeshFactory* meshFactory
) const
{
	Ref< StreamMesh > streamMesh = new StreamMesh();
	streamMesh->m_stream = dataStream;
	streamMesh->m_meshReader = new render::MeshReader(meshFactory);
	streamMesh->m_frameOffsets = m_frameOffsets;
	streamMesh->m_boundingBox = m_boundingBox;

	for (std::vector< StreamMeshResource::Part >::const_iterator i = m_parts.begin(); i != m_parts.end(); ++i)
	{
		StreamMesh::Part part;
		part.material = i->material;
		part.opaque = i->opaque;

		if (resourceManager->bind(part.material))
			streamMesh->m_parts[i->name] = part;
		else
			return 0;
	}

	return streamMesh;
}

bool StreamMeshResource::serialize(ISerializer& s)
{
	s >> MemberStlVector< uint32_t >(L"frameOffsets", m_frameOffsets);
	s >> Member< Vector4 >(L"boundingBoxMin", m_boundingBox.mn);
	s >> Member< Vector4 >(L"boundingBoxMax", m_boundingBox.mx);
	s >> MemberStlVector< Part, MemberComposite< Part > >(L"parts", m_parts);
	return true;
}

StreamMeshResource::Part::Part()
:	opaque(true)
{
}

bool StreamMeshResource::Part::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", name);
	s >> Member< Guid >(L"material", material);
	s >> Member< bool >(L"opaque", opaque);
	return true;
}

	}
}
