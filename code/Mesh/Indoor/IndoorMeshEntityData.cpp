#include "Mesh/Indoor/IndoorMeshEntityData.h"
#include "Mesh/Indoor/IndoorMeshEntity.h"
#include "Mesh/Indoor/IndoorMesh.h"
#include "Mesh/Indoor/IndoorMeshResource.h"
#include "Core/Serialization/ISerializer.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.mesh.IndoorMeshEntityData", IndoorMeshEntityData, MeshEntityData)

Ref< MeshEntity > IndoorMeshEntityData::createEntity(resource::IResourceManager* resourceManager, world::IEntityBuilder* builder) const
{
	if (!resourceManager->bind(m_mesh))
		return 0;

	return new IndoorMeshEntity(
		getTransform(),
		m_mesh
	);
}

bool IndoorMeshEntityData::serialize(ISerializer& s)
{
	if (!MeshEntityData::serialize(s))
		return false;
	return s >> resource::Member< IndoorMesh, IndoorMeshResource >(L"mesh", m_mesh);
}

	}
}
