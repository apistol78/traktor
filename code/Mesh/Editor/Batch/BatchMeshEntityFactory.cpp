#include "Mesh/Editor/Batch/BatchMeshEntityData.h"
#include "Mesh/Editor/Batch/BatchMeshEntityFactory.h"
#include "World/IEntityBuilder.h"
#include "World/Entity/GroupEntity.h"

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

const TypeInfoSet BatchMeshEntityFactory::getEntityEventTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet BatchMeshEntityFactory::getEntityComponentTypes() const
{
	return TypeInfoSet();
}

Ref< world::Entity > BatchMeshEntityFactory::createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const
{
	const BatchMeshEntityData* batchMeshEntityData = checked_type_cast< const BatchMeshEntityData* >(&entityData);
	
	Ref< world::GroupEntity > batchEntity = new world::GroupEntity(batchMeshEntityData->getTransform());

	const RefArray< world::EntityData >& childEntityData = batchMeshEntityData->getEntityData();
	for (RefArray< world::EntityData >::const_iterator i = childEntityData.begin(); i != childEntityData.end(); ++i)
	{
		Ref< world::Entity > childEntity = builder->create(*i);
		if (childEntity)
			batchEntity->addEntity(childEntity);
	}

	return batchEntity;
}

Ref< world::IEntityEvent > BatchMeshEntityFactory::createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const
{
	return 0;
}

Ref< world::IEntityComponent > BatchMeshEntityFactory::createEntityComponent(const world::IEntityBuilder* builder, world::Entity* owner, const world::IEntityComponentData& entityComponentData) const
{
	return 0;
}

	}
}
