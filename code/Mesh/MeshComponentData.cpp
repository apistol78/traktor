#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Mesh/MeshComponentData.h"
#include "Mesh/Blend/BlendMesh.h"
#include "Mesh/Blend/BlendMeshComponent.h"
#include "Mesh/Indoor/IndoorMesh.h"
#include "Mesh/Indoor/IndoorMeshComponent.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Mesh/Instance/InstanceMeshComponent.h"
#include "Mesh/Lod/AutoLodMesh.h"
#include "Mesh/Lod/AutoLodMeshComponent.h"
#include "Mesh/Partition/PartitionMesh.h"
#include "Mesh/Partition/PartitionMeshComponent.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Mesh/Skinned/SkinnedMeshComponent.h"
#include "Mesh/Static/StaticMesh.h"
#include "Mesh/Static/StaticMeshComponent.h"
#include "Mesh/Stream/StreamMesh.h"
#include "Mesh/Stream/StreamMeshComponent.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.mesh.MeshComponentData", 0, MeshComponentData, world::IEntityComponentData)

MeshComponentData::MeshComponentData()
:	m_screenSpaceCulling(false)
{
}

MeshComponentData::MeshComponentData(const resource::Id< IMesh >& mesh)
:	m_mesh(mesh)
,	m_screenSpaceCulling(false)
{
}

Ref< MeshComponent > MeshComponentData::createComponent(resource::IResourceManager* resourceManager) const
{
	resource::Proxy< IMesh > mesh;
	if (!resourceManager->bind(m_mesh, mesh))
		return 0;

	Ref< MeshComponent > component;

	if (is_a< BlendMesh >(mesh.getResource()))
		component = new BlendMeshComponent(resource::Proxy< BlendMesh >(mesh.getHandle()), m_screenSpaceCulling);
	else if (is_a< IndoorMesh >(mesh.getResource()))
		component = new IndoorMeshComponent(resource::Proxy< IndoorMesh >(mesh.getHandle()), m_screenSpaceCulling);
	else if (is_a< InstanceMesh >(mesh.getResource()))
		component = new InstanceMeshComponent(resource::Proxy< InstanceMesh >(mesh.getHandle()), m_screenSpaceCulling);
	else if (is_a< AutoLodMesh >(mesh.getResource()))
		component = new AutoLodMeshComponent(resource::Proxy< AutoLodMesh >(mesh.getHandle()), m_screenSpaceCulling);
	else if (is_a< PartitionMesh >(mesh.getResource()))
		component = new PartitionMeshComponent(resource::Proxy< PartitionMesh >(mesh.getHandle()), m_screenSpaceCulling);
	else if (is_a< SkinnedMesh >(mesh.getResource()))
		component = new SkinnedMeshComponent(resource::Proxy< SkinnedMesh >(mesh.getHandle()), m_screenSpaceCulling);
	else if (is_a< StaticMesh >(mesh.getResource()))
		component = new StaticMeshComponent(resource::Proxy< StaticMesh >(mesh.getHandle()), m_screenSpaceCulling);
	else if (is_a< StreamMesh >(mesh.getResource()))
		component = new StreamMeshComponent(resource::Proxy< StreamMesh >(mesh.getHandle()), m_screenSpaceCulling);

	return component;
}

void MeshComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< IMesh >(L"mesh", m_mesh);
	s >> Member< bool >(L"screenSpaceCulling", m_screenSpaceCulling);
}

	}
}
