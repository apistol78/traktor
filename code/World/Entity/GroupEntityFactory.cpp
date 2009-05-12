#include "World/Entity/GroupEntityFactory.h"
#include "World/Entity/GroupEntityData.h"
#include "World/Entity/GroupEntity.h"
#include "World/Entity/SpatialGroupEntityData.h"
#include "World/Entity/SpatialGroupEntity.h"
#include "World/Entity/EntityBuilder.h"

namespace traktor
{
	namespace world
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.world.GroupEntityFactory", GroupEntityFactory, EntityFactory)

const TypeSet GroupEntityFactory::getEntityTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< GroupEntityData >());
	typeSet.insert(&type_of< SpatialGroupEntityData >());
	return typeSet;
}

Entity* GroupEntityFactory::createEntity(EntityBuilder* builder, const EntityData& entityData) const
{
	Ref< Entity > entity;

	if (const GroupEntityData* groupData = dynamic_type_cast< const GroupEntityData* >(&entityData))
	{
		const RefArray< EntityData >& childData = groupData->getEntityData();
		if (childData.empty())
			return 0;

		Ref< GroupEntity > groupEntity = gc_new< GroupEntity >();
		
		groupEntity->m_name = entityData.getName();

		for (RefArray< EntityData >::const_iterator i = childData.begin(); i != childData.end(); ++i)
		{
			Ref< Entity > child = builder->build(*i);
			if (child)
				groupEntity->m_entities.push_back(child);
		}

		entity = groupEntity;
	}
	else if (const SpatialGroupEntityData* groupData = dynamic_type_cast< const SpatialGroupEntityData* >(&entityData))
	{
		const RefArray< SpatialEntityData >& childData = groupData->getEntityData();
		if (childData.empty())
			return 0;

		Ref< SpatialGroupEntity > groupEntity = gc_new< SpatialGroupEntity >(
			cref(groupData->getTransform())
		);

		groupEntity->m_name = entityData.getName();

		for (RefArray< SpatialEntityData >::const_iterator i = childData.begin(); i != childData.end(); ++i)
		{
			Ref< SpatialEntity > child = dynamic_type_cast< SpatialEntity* >(builder->build(*i));
			if (child)
				groupEntity->m_entities.push_back(child);
		}

		entity = groupEntity;
	}

	return entity;
}
	
	}
}
