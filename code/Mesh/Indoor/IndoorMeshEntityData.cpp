#include "Mesh/Indoor/IndoorMeshEntityData.h"
#include "Mesh/Indoor/IndoorMeshEntity.h"
#include "Mesh/Indoor/IndoorMesh.h"
#include "Mesh/Indoor/IndoorMeshResource.h"
#include "Core/Serialization/Serializer.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.mesh.IndoorMeshEntityData", IndoorMeshEntityData, MeshEntityData)

MeshEntity* IndoorMeshEntityData::createEntity(world::EntityBuilder* builder) const
{
	return gc_new< IndoorMeshEntity >(getTransform(), m_mesh);
}

bool IndoorMeshEntityData::serialize(Serializer& s)
{
	if (!MeshEntityData::serialize(s))
		return false;
	return s >> resource::Member< IndoorMesh, IndoorMeshResource >(L"mesh", m_mesh);
}

	}
}
