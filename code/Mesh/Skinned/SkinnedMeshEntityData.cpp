#include "Mesh/Skinned/SkinnedMeshEntityData.h"
#include "Mesh/Skinned/SkinnedMeshEntity.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Mesh/Skinned/SkinnedMeshResource.h"
#include "Core/Serialization/Serializer.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.mesh.SkinnedMeshEntityData", SkinnedMeshEntityData, MeshEntityData)

MeshEntity* SkinnedMeshEntityData::createEntity(world::EntityBuilder* builder) const
{
	return gc_new< SkinnedMeshEntity >(
		cref(getTransform()),
		m_mesh
	);
}

bool SkinnedMeshEntityData::serialize(Serializer& s)
{
	if (!MeshEntityData::serialize(s))
		return false;
	return s >> resource::Member< SkinnedMesh, SkinnedMeshResource >(L"mesh", m_mesh);
}

	}
}
