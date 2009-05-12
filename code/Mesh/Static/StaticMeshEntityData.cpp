#include "Mesh/Static/StaticMeshEntityData.h"
#include "Mesh/Static/StaticMeshEntity.h"
#include "Mesh/Static/StaticMesh.h"
#include "Mesh/Static/StaticMeshResource.h"
#include "Core/Serialization/Serializer.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.mesh.StaticMeshEntityData", StaticMeshEntityData, MeshEntityData)

MeshEntity* StaticMeshEntityData::createEntity(world::EntityBuilder* builder) const
{
	return gc_new< StaticMeshEntity >(getTransform(), m_mesh);
}

bool StaticMeshEntityData::serialize(Serializer& s)
{
	if (!MeshEntityData::serialize(s))
		return false;
	return s >> resource::Member< StaticMesh, StaticMeshResource >(L"mesh", m_mesh);
}

	}
}
