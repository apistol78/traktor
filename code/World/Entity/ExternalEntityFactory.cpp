#include "World/Entity/ExternalEntityFactory.h"
#include "World/Entity/ExternalEntityData.h"
#include "World/Entity/ExternalSpatialEntityData.h"
#include "World/Entity/EntityBuilder.h"
#include "Database/Database.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.ExternalEntityFactory", ExternalEntityFactory, EntityFactory)

ExternalEntityFactory::ExternalEntityFactory(db::Database* database)
:	m_database(database)
{
}

const TypeSet ExternalEntityFactory::getEntityTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< ExternalEntityData >());
	typeSet.insert(&type_of< ExternalSpatialEntityData >());
	return typeSet;
}

Entity* ExternalEntityFactory::createEntity(EntityBuilder* builder, const EntityData& entityData) const
{
	Ref< EntityData > realEntityData;

	if (const ExternalEntityData* externalEntityData = dynamic_type_cast< const ExternalEntityData* >(&entityData))
	{
		realEntityData = m_database->getObjectReadOnly< EntityData >(externalEntityData->getGuid());
	}
	else if (const ExternalSpatialEntityData* externalSpatialEntityData = dynamic_type_cast< const ExternalSpatialEntityData* >(&entityData))
	{
		Ref< SpatialEntityData > realSpatialEntityData = m_database->getObjectReadOnly< SpatialEntityData >(externalSpatialEntityData->getGuid());
		if (realSpatialEntityData)
		{
			realSpatialEntityData->setTransform(externalSpatialEntityData->getTransform());
			realEntityData = realSpatialEntityData;
		}
	}

	if (!realEntityData)
		return 0;

	return builder->build(realEntityData);
}

	}
}
