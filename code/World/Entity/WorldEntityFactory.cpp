#include "Database/Database.h"
#include "World/Entity/DirectionalLightEntity.h"
#include "World/Entity/DirectionalLightEntityData.h"
#include "World/Entity/ExternalEntityData.h"
#include "World/Entity/ExternalEntityDataCache.h"
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

WorldEntityFactory::WorldEntityFactory(db::Database* database, ExternalEntityDataCache* externalCache)
:	m_database(database)
,	m_externalCache(externalCache)
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

Ref< Entity > WorldEntityFactory::createEntity(IEntityBuilder* builder, const EntityData& entityData) const
{
	if (const ExternalEntityData* externalEntityData = dynamic_type_cast< const ExternalEntityData* >(&entityData))
	{
		Guid entityGuid = externalEntityData->getGuid();
		Ref< EntityData > resolvedEntityData;

		if (m_externalCache)
			resolvedEntityData = m_externalCache->get(entityGuid);

		if (!resolvedEntityData)
		{
			resolvedEntityData = m_database->getObjectReadOnly< EntityData >(entityGuid);
			if (m_externalCache && resolvedEntityData)
				m_externalCache->put(entityGuid, resolvedEntityData);
		}

		if (resolvedEntityData)
			resolvedEntityData->setName(externalEntityData->getName());

		return builder->create(resolvedEntityData);
	}

	if (const ExternalSpatialEntityData* externalSpatialEntityData = dynamic_type_cast< const ExternalSpatialEntityData* >(&entityData))
	{
		Guid entityGuid = externalSpatialEntityData->getGuid();
		Ref< SpatialEntityData > resolvedEntityData;

		if (m_externalCache)
			resolvedEntityData = dynamic_type_cast< SpatialEntityData* >(m_externalCache->get(entityGuid));

		if (!resolvedEntityData)
		{
			resolvedEntityData = m_database->getObjectReadOnly< SpatialEntityData >(entityGuid);
			if (m_externalCache && resolvedEntityData)
				m_externalCache->put(entityGuid, resolvedEntityData);
		}

		if (resolvedEntityData)
		{
			resolvedEntityData->setName(externalSpatialEntityData->getName());
			resolvedEntityData->setTransform(externalSpatialEntityData->getTransform());
		}

		return builder->create(resolvedEntityData);
	}

	if (const GroupEntityData* groupData = dynamic_type_cast< const GroupEntityData* >(&entityData))
	{
		Ref< GroupEntity > groupEntity = new GroupEntity();

		const RefArray< EntityData >& entityData = groupData->getEntityData();
		for (RefArray< EntityData >::const_iterator i = entityData.begin(); i != entityData.end(); ++i)
		{
			Ref< Entity > childEntity = builder->create(*i);
			if (childEntity)
				groupEntity->m_entities.push_back(childEntity);
		}

		return groupEntity;
	}

	if (const SpatialGroupEntityData* spatialGroupData = dynamic_type_cast< const SpatialGroupEntityData* >(&entityData))
	{
		Ref< SpatialGroupEntity > spatialGroupEntity = new SpatialGroupEntity(spatialGroupData->getTransform());

		const RefArray< SpatialEntityData >& entityData = spatialGroupData->getEntityData();
		for (RefArray< SpatialEntityData >::const_iterator i = entityData.begin(); i != entityData.end(); ++i)
		{
			Ref< SpatialEntity > childEntity = dynamic_type_cast< SpatialEntity* >(builder->create(*i));
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
