#include "Mesh/Blend/BlendMeshEntityData.h"
#include "Mesh/Blend/BlendMeshEntity.h"
#include "Mesh/Blend/BlendMesh.h"
#include "Mesh/Blend/BlendMeshResource.h"
#include "Resource/IResourceManager.h"
#include "Core/Serialization/Serializer.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_EDITABLE_CLASS(L"traktor.mesh.BlendMeshEntityData", BlendMeshEntityData, MeshEntityData)

Ref< MeshEntity > BlendMeshEntityData::createEntity(resource::IResourceManager* resourceManager, world::IEntityBuilder* builder) const
{
	if (!resourceManager->bind(m_mesh))
		return 0;

	return gc_new< BlendMeshEntity >(
		cref(getTransform()),
		m_mesh
	);
}

bool BlendMeshEntityData::serialize(Serializer& s)
{
	if (!MeshEntityData::serialize(s))
		return false;
	return s >> resource::Member< BlendMesh, BlendMeshResource >(L"mesh", m_mesh);
}

	}
}
