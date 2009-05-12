#include "Mesh/Blend/BlendMeshEntityData.h"
#include "Mesh/Blend/BlendMeshEntity.h"
#include "Mesh/Blend/BlendMesh.h"
#include "Mesh/Blend/BlendMeshResource.h"
#include "Core/Serialization/Serializer.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.mesh.BlendMeshEntityData", BlendMeshEntityData, MeshEntityData)

MeshEntity* BlendMeshEntityData::createEntity(world::EntityBuilder* builder) const
{
	return gc_new< BlendMeshEntity >(getTransform(), m_mesh);
}

bool BlendMeshEntityData::serialize(Serializer& s)
{
	if (!MeshEntityData::serialize(s))
		return false;
	return s >> resource::Member< BlendMesh, BlendMeshResource >(L"mesh", m_mesh);
}

	}
}
