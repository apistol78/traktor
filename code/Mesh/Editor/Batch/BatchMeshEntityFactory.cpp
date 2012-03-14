#include "Mesh/Editor/Batch/BatchMeshEntityData.h"
#include "Mesh/Editor/Batch/BatchMeshEntityFactory.h"
#include "World/Entity/IEntityBuilder.h"
#include "World/Entity/SpatialGroupEntity.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.BatchMeshEntityFactory", BatchMeshEntityFactory, world::IEntityFactory)

BatchMeshEntityFactory::BatchMeshEntityFactory(resource::IResourceManager* resourceManager)
:	m_resourceManager(resourceManager)
{
}

const TypeInfoSet BatchMeshEntityFactory::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< BatchMeshEntityData >());
	return typeSet;
}

Ref< world::Entity > BatchMeshEntityFactory::createEntity(world::IEntityBuilder* builder, const world::EntityData& entityData) const
{
	const BatchMeshEntityData* batchMeshEntityData = checked_type_cast< const BatchMeshEntityData* >(&entityData);
	
	Ref< world::SpatialGroupEntity > batchEntity = new world::SpatialGroupEntity(batchMeshEntityData->getTransform());

	const RefArray< world::SpatialEntityData >& childEntityData = batchMeshEntityData->getEntityData();
	for (RefArray< world::SpatialEntityData >::const_iterator i = childEntityData.begin(); i != childEntityData.end(); ++i)
	{
		Ref< world::SpatialEntity > childEntity = dynamic_type_cast< world::SpatialEntity* >(builder->create(*i));
		if (childEntity)
			batchEntity->addEntity(childEntity);
	}

	return batchEntity;
}

	}
}
