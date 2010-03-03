#include "Core/Serialization/ISerializer.h"
#include "Mesh/IMesh.h"
#include "Mesh/IMeshResource.h"
#include "Mesh/MeshEntityData.h"
#include "Mesh/Blend/BlendMesh.h"
#include "Mesh/Blend/BlendMeshEntity.h"
#include "Mesh/Indoor/IndoorMesh.h"
#include "Mesh/Indoor/IndoorMeshEntity.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Mesh/Instance/InstanceMeshEntity.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Mesh/Skinned/SkinnedMeshEntity.h"
#include "Mesh/Static/StaticMesh.h"
#include "Mesh/Static/StaticMeshEntity.h"
#include "Mesh/Stream/StreamMesh.h"
#include "Mesh/Stream/StreamMeshEntity.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.MeshEntityData", 0, MeshEntityData, AbstractMeshEntityData)

Ref< MeshEntity > MeshEntityData::createEntity(resource::IResourceManager* resourceManager, world::IEntityBuilder* builder) const
{
	if (!resourceManager->bind(m_mesh))
		return 0;

	Ref< MeshEntity > meshEntity;

	if (BlendMesh* blendMesh = dynamic_type_cast< BlendMesh* >(m_mesh))
		meshEntity = new BlendMeshEntity(getTransform(), blendMesh);
	else if (IndoorMesh* indoorMesh = dynamic_type_cast< IndoorMesh* >(m_mesh))
		meshEntity = new IndoorMeshEntity(getTransform(), indoorMesh);
	else if (InstanceMesh* instanceMesh = dynamic_type_cast< InstanceMesh* >(m_mesh))
		meshEntity = new InstanceMeshEntity(getTransform(), instanceMesh);
	else if (SkinnedMesh* skinnedMesh = dynamic_type_cast< SkinnedMesh* >(m_mesh))
		meshEntity = new SkinnedMeshEntity(getTransform(), skinnedMesh);
	else if (StaticMesh* staticMesh = dynamic_type_cast< StaticMesh* >(m_mesh))
		meshEntity = new StaticMeshEntity(getTransform(), staticMesh);
	else if (StreamMesh* streamMesh = dynamic_type_cast< StreamMesh* >(m_mesh))
		meshEntity = new StreamMeshEntity(getTransform(), streamMesh);

	return meshEntity;
}

bool MeshEntityData::serialize(ISerializer& s)
{
	if (!AbstractMeshEntityData::serialize(s))
		return false;

	return s >> resource::Member< IMesh, IMeshResource >(L"mesh", m_mesh);
}

	}
}
