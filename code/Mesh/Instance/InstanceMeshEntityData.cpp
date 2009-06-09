#include "Mesh/Instance/InstanceMeshEntityData.h"
#include "Mesh/Instance/InstanceMeshEntity.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Mesh/Instance/InstanceMeshResource.h"
#include "Core/Serialization/Serializer.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.mesh.InstanceMeshEntityData", InstanceMeshEntityData, MeshEntityData)

MeshEntity* InstanceMeshEntityData::createEntity(world::EntityBuilder* builder) const
{
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
