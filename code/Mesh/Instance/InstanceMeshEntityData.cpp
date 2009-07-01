#include "Mesh/Instance/InstanceMeshEntityData.h"
#include "Mesh/Instance/InstanceMeshEntity.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Mesh/Instance/InstanceMeshResource.h"
#include "Resource/IResourceManager.h"
#include "Core/Serialization/Serializer.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_EDITABLE_CLASS(L"traktor.mesh.InstanceMeshEntityData", InstanceMeshEntityData, MeshEntityData)

MeshEntity* InstanceMeshEntityData::createEntity(resource::IResourceManager* resourceManager, world::IEntityBuilder* builder) const
{
	if (!resourceManager->bind(m_mesh))
		return 0;

	return gc_new< InstanceMeshEntity >(
		cref(getTransform()),
		m_mesh
	);
}

bool InstanceMeshEntityData::serialize(Serializer& s)
{
	if (!MeshEntityData::serialize(s))
		return false;
	return s >> resource::Member< InstanceMesh, InstanceMeshResource >(L"mesh", m_mesh);
}

	}
}
