#include "Core/Serialization/ISerializer.h"
#include "Mesh/Stream/StreamMesh.h"
#include "Mesh/Stream/StreamMeshEntity.h"
#include "Mesh/Stream/StreamMeshEntityData.h"
#include "Mesh/Stream/StreamMeshResource.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.mesh.StreamMeshEntityData", 0, StreamMeshEntityData, MeshEntityData)

Ref< MeshEntity > StreamMeshEntityData::createEntity(resource::IResourceManager* resourceManager, world::IEntityBuilder* builder) const
{
	if (!resourceManager->bind(m_mesh))
		return 0;

	return new StreamMeshEntity(
		getTransform(),
		m_mesh
	);
}

bool StreamMeshEntityData::serialize(ISerializer& s)
{
	if (!MeshEntityData::serialize(s))
		return false;
	return s >> resource::Member< StreamMesh, StreamMeshResource >(L"mesh", m_mesh);
}

	}
}
