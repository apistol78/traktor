#include "Mesh/Static/StaticMeshEntityData.h"
#include "Mesh/Static/StaticMeshEntity.h"
#include "Mesh/Static/StaticMesh.h"
#include "Mesh/Static/StaticMeshResource.h"
#include "Core/Serialization/Serializer.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_EDITABLE_CLASS(L"traktor.mesh.StaticMeshEntityData", StaticMeshEntityData, MeshEntityData)

Ref< MeshEntity > StaticMeshEntityData::createEntity(resource::IResourceManager* resourceManager, world::IEntityBuilder* builder) const
{
	if (!resourceManager->bind(m_mesh))
		return 0;

	return gc_new< StaticMeshEntity >(
		cref(getTransform()),
		m_mesh
	);
}

bool StaticMeshEntityData::serialize(Serializer& s)
{
	if (!MeshEntityData::serialize(s))
		return false;
	return s >> resource::Member< StaticMesh, StaticMeshResource >(L"mesh", m_mesh);
}

	}
}
