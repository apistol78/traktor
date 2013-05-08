#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAabb.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStl.h"
#include "Mesh/Partition/IPartitionData.h"
#include "Mesh/Partition/PartitionMesh.h"
#include "Mesh/Partition/PartitionMeshResource.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/MeshReader.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.PartitionMeshResource", 1, PartitionMeshResource, IMeshResource)

Ref< IMesh > PartitionMeshResource::createMesh(
	const std::wstring& name,
	IStream* dataStream,
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	render::MeshFactory* meshFactory
) const
{
	T_ASSERT (m_partitionData);

	Ref< render::Mesh > mesh = render::MeshReader(meshFactory).read(dataStream);
	if (!mesh)
	{
		log::error << L"Partition mesh create failed; unable to read mesh" << Endl;
		return 0;
	}

	Ref< PartitionMesh > partitionMesh = new PartitionMesh();
	partitionMesh->m_mesh = mesh;
	partitionMesh->m_parts.reserve(m_parts.size());

	for (AlignedVector< Part >::const_iterator i = m_parts.begin(); i != m_parts.end(); ++i)
	{
		PartitionMesh::Part part;
		part.shaderTechnique = render::getParameterHandle(i->shaderTechnique);
		part.meshPart = i->meshPart;
		part.boundingBox = i->boundingBox;
		partitionMesh->m_parts.push_back(part);
	}

	if (!resourceManager->bind(m_shader, partitionMesh->m_shader))
		return 0;

	partitionMesh->m_partition = m_partitionData->createPartition();
	if (!partitionMesh->m_partition)
		return 0;
	
#if defined(_DEBUG)
	partitionMesh->m_name = wstombs(name);
#endif

	return partitionMesh;
}

void PartitionMeshResource::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> MemberAlignedVector< Part, MemberComposite< Part > >(L"parts", m_parts);
	s >> MemberRef< IPartitionData >(L"partitionData", m_partitionData);
}

PartitionMeshResource::Part::Part()
:	meshPart(0)
{
}

void PartitionMeshResource::Part::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"shaderTechnique", shaderTechnique);
	s >> Member< uint32_t >(L"meshPart", meshPart);
	s >> MemberAabb(L"boundingBox", boundingBox);
}

	}
}
