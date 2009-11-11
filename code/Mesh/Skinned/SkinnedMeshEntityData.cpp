#include "Mesh/Skinned/SkinnedMeshEntityData.h"
#include "Mesh/Skinned/SkinnedMeshEntity.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Mesh/Skinned/SkinnedMeshResource.h"
#include "Core/Serialization/Serializer.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_EDITABLE_CLASS(L"traktor.mesh.SkinnedMeshEntityData", SkinnedMeshEntityData, MeshEntityData)

Ref< MeshEntity > SkinnedMeshEntityData::createEntity(resource::IResourceManager* resourceManager, world::IEntityBuilder* builder) const
{
	if (!resourceManager->bind(m_mesh))
		return 0;

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
