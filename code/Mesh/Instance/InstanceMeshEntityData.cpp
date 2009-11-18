#include "Mesh/Instance/InstanceMeshEntityData.h"
#include "Mesh/Instance/InstanceMeshEntity.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Mesh/Instance/InstanceMeshResource.h"
#include "Resource/IResourceManager.h"
#include "Core/Serialization/ISerializer.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.mesh.InstanceMeshEntityData", 0, InstanceMeshEntityData, MeshEntityData)

Ref< MeshEntity > InstanceMeshEntityData::createEntity(resource::IResourceManager* resourceManager, world::IEntityBuilder* builder) const
{
	if (!resourceManager->bind(m_mesh))
		return 0;

	return new InstanceMeshEntity(
		getTransform(),
		m_mesh
	);
}

bool InstanceMeshEntityData::serialize(ISerializer& s)
{
	if (!MeshEntityData::serialize(s))
		return false;
	return s >> resource::Member< InstanceMesh, InstanceMeshResource >(L"mesh", m_mesh);
}

	}
}
