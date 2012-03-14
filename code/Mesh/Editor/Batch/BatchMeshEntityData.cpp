//#include "Core/Log/Log.h"
//#include "Core/Misc/String.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Mesh/Editor/Batch/BatchMeshEntityData.h"
#include "World/Entity/SpatialEntityData.h"
//#include "World/Entity/IEntityBuilder.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.mesh.BatchMeshEntityData", 0, BatchMeshEntityData, world::SpatialEntityData)

BatchMeshEntityData::BatchMeshEntityData()
:	m_outputGuid(Guid::create())
{
}

//Ref< MeshEntity > BatchMeshEntityData::createEntity(resource::IResourceManager* resourceManager, world::IEntityBuilder* builder) const
//{
//	Ref< CompositeMeshEntity > batchMeshEntity = new CompositeMeshEntity(getTransform());
//	for (RefArray< world::SpatialEntityData >::const_iterator i = m_entityData.begin(); i != m_entityData.end(); ++i)
//	{
//		Ref< MeshEntity > meshEntity = dynamic_type_cast< MeshEntity* >(builder->create(*i));
//		if (meshEntity)
//			batchMeshEntity->addMeshEntity((*i)->getName() + L"_" + toString(std::distance(m_entityData.begin(), i)), meshEntity);
//		else
//			log::error << L"Unable to create composite mesh entity part" << Endl;
//	}
//	return batchMeshEntity;
//}

void BatchMeshEntityData::setTransform(const Transform& transform)
{
	Transform deltaTransform = getTransform().inverse() * transform;
	for (RefArray< world::SpatialEntityData >::iterator i = m_entityData.begin(); i != m_entityData.end(); ++i)
	{
		Transform currentTransform = (*i)->getTransform();
		(*i)->setTransform(currentTransform * deltaTransform);
	}
	world::SpatialEntityData::setTransform(transform);
}

bool BatchMeshEntityData::serialize(ISerializer& s)
{
	if (!world::SpatialEntityData::serialize(s))
		return false;
	
	s >> Member< Guid >(L"outputGuid", m_outputGuid);
	s >> MemberRefArray< world::SpatialEntityData >(L"entityData", m_entityData);

	return true;
}

	}
}
