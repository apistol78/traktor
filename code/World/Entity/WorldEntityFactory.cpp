#include "Database/Database.h"
#include "World/Entity/DirectionalLightEntity.h"
#include "World/Entity/DirectionalLightEntityData.h"
#include "World/Entity/ExternalEntityData.h"
#include "World/Entity/ExternalSpatialEntityData.h"
#include "World/Entity/GroupEntity.h"
#include "World/Entity/GroupEntityData.h"
#include "World/Entity/IEntityBuilder.h"
#include "World/Entity/NullEntity.h"
#include "World/Entity/NullEntityData.h"
#include "World/Entity/PointLightEntity.h"
#include "World/Entity/PointLightEntityData.h"
#include "World/Entity/SpatialGroupEntity.h"
#include "World/Entity/SpatialGroupEntityData.h"
#include "World/Entity/WorldEntityFactory.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldEntityFactory", WorldEntityFactory, IEntityFactory)

WorldEntityFactory::WorldEntityFactory(db::Database* database)
:	m_database(database)
{
}

const TypeInfoSet WorldEntityFactory::getEntityTypes() const
{
	TypeInfoSet typeSet;

	typeSet.insert(&type_of< ExternalEntityData >());
	typeSet.insert(&type_of< ExternalSpatialEntityData >());

	typeSet.insert(&type_of< GroupEntityData >());
	typeSet.insert(&type_of< SpatialGroupEntityData >());

	typeSet.insert(&type_of< DirectionalLightEntityData >());
	typeSet.insert(&type_of< PointLightEntityData >());

	typeSet.insert(&type_of< NullEntityData >());
	return typeSet;
}

Ref< Entity > WorldEntityFactory::createEntity(IEntityBuilder* builder, const std::wstring& name, const EntityData& entityData, const Object* instanceData) const
{
	if (const ExternalEntityData* externalEntityData = dynamic_type_cast< const ExternalEntityData* >(&entityData))
	{
		Guid entityGuid = externalEntityData->getGuid();
		Ref< EntityData > resolvedEntityData = m_database->getObjectReadOnly< EntityData >(entityGuid);
		return builder->create(name, resolvedEntityData, instanceData);
	}

	if (const ExternalSpatialEntityData* externalSpatialEntityData = dynamic_type_cast< const ExternalSpatialEntityData* >(&entityData))
	{
		Guid entityGuid = externalSpatialEntityData->getGuid();
		Ref< SpatialEntityData > resolvedEntityData = m_database->getObjectReadOnly< SpatialEntityData >(entityGuid);
		if (resolvedEntityData)
			resolvedEntityData->setTransform(externalSpatialEntityData->getTransform());
		return builder->create(name, resolvedEntityData, instanceData);
	}

	if (const GroupEntityData* groupData = dynamic_type_cast< const GroupEntityData* >(&entityData))
	{
		const RefArray< EntityInstance >& instances = groupData->getInstances();
		Ref< GroupEntity > groupEntity = new GroupEntity();
		for (RefArray< EntityInstance >::const_iterator i = instances.begin(); i != instances.end(); ++i)
		{
			Ref< Entity > childEntity = builder->build(*i);
			if (childEntity)
				groupEntity->m_entities.push_back(childEntity);
		}
		return groupEntity;
	}

	if (const SpatialGroupEntityData* spatialGroupData = dynamic_type_cast< const SpatialGroupEntityData* >(&entityData))
	{
		const RefArray< EntityInstance >& instances = spatialGroupData->getInstances();
		Ref< SpatialGroupEntity > spatialGroupEntity = new SpatialGroupEntity(spatialGroupData->getTransform());
		for (RefArray< EntityInstance >::const_iterator i = instances.begin(); i != instances.end(); ++i)
		{
			Ref< SpatialEntity > childEntity = dynamic_type_cast< SpatialEntity* >(builder->build(*i));
			if (childEntity)
				spatialGroupEntity->m_entities.push_back(childEntity);
		}
		return spatialGroupEntity;
	}

	if (const DirectionalLightEntityData* directionalLightData = dynamic_type_cast< const DirectionalLightEntityData* >(&entityData))
	{
		return new DirectionalLightEntity(
			directionalLightData->getTransform(),
			directionalLightData->getSunColor(),
			directionalLightData->getBaseColor(),
			directionalLightData->getShadowColor()
		);
	}

	if (const PointLightEntityData* pointLightData = dynamic_type_cast< const PointLightEntityData* >(&entityData))
	{
		return new PointLightEntity(
			pointLightData->getTransform(),
			pointLightData->getSunColor(),
			pointLightData->getBaseColor(),
			pointLightData->getShadowColor(),
			pointLightData->getRange(),
			pointLightData->getRandomFlicker()
		);
	}

	if (const NullEntityData* nullData = dynamic_type_cast< const NullEntityData* >(&entityData))
	{
		return new NullEntity(nullData->getTransform());
	}

	return 0;
}

	}
}
