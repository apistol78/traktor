#include "Core/Serialization/ISerializer.h"
#include "Mesh/IMesh.h"
#include "Mesh/IMeshResource.h"
#include "Mesh/MeshEntityData.h"
#include "Mesh/Blend/BlendMesh.h"
#include "Mesh/Blend/BlendMeshEntity.h"
#include "Mesh/Indoor/IndoorMesh.h"
#include "Mesh/Indoor/IndoorMeshEntity.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Mesh/Instance/InstanceMeshEntity.h"
#include "Mesh/Partition/PartitionMesh.h"
#include "Mesh/Partition/PartitionMeshEntity.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Mesh/Skinned/SkinnedMeshEntity.h"
#include "Mesh/Static/StaticMesh.h"
#include "Mesh/Static/StaticMeshEntity.h"
#include "Mesh/Stream/StreamMesh.h"
#include "Mesh/Stream/StreamMeshEntity.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.mesh.MeshEntityData", 0, MeshEntityData, AbstractMeshEntityData)

Ref< MeshEntity > MeshEntityData::createEntity(resource::IResourceManager* resourceManager, world::IEntityBuilder* builder) const
{
	if (!resourceManager->bind(m_mesh))
		return 0;

	Ref< MeshEntity > meshEntity;

	if (is_a< BlendMesh >(m_mesh))
		meshEntity = new BlendMeshEntity(getTransform(), resource::Proxy< BlendMesh >(m_mesh.getHandle()));
	else if (is_a< IndoorMesh >(m_mesh))
		meshEntity = new IndoorMeshEntity(getTransform(), resource::Proxy< IndoorMesh >(m_mesh.getHandle()));
	else if (is_a< InstanceMesh >(m_mesh))
		meshEntity = new InstanceMeshEntity(getTransform(), resource::Proxy< InstanceMesh >(m_mesh.getHandle()));
	else if (is_a< PartitionMesh >(m_mesh))
		meshEntity = new PartitionMeshEntity(getTransform(), resource::Proxy< PartitionMesh >(m_mesh.getHandle()));
	else if (is_a< SkinnedMesh >(m_mesh))
		meshEntity = new SkinnedMeshEntity(getTransform(), resource::Proxy< SkinnedMesh >(m_mesh.getHandle()));
	else if (is_a< StaticMesh >(m_mesh))
		meshEntity = new StaticMeshEntity(getTransform(), resource::Proxy< StaticMesh >(m_mesh.getHandle()));
	else if (is_a< StreamMesh >(m_mesh))
		meshEntity = new StreamMeshEntity(getTransform(), resource::Proxy< StreamMesh >(m_mesh.getHandle()));

	return meshEntity;
}

bool MeshEntityData::serialize(ISerializer& s)
{
	if (!AbstractMeshEntityData::serialize(s))
		return false;

	return s >> resource::Member< IMesh, IMeshResource >(L"mesh", m_mesh);
}

	}
}
