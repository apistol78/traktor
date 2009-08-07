#include "World/Entity/GroupEntityFactory.h"
#include "World/Entity/GroupEntityData.h"
#include "World/Entity/GroupEntity.h"
#include "World/Entity/SpatialGroupEntityData.h"
#include "World/Entity/SpatialGroupEntity.h"
#include "World/Entity/IEntityBuilder.h"

namespace traktor
{
	namespace world
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.world.GroupEntityFactory", GroupEntityFactory, IEntityFactory)

const TypeSet GroupEntityFactory::getEntityTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< GroupEntityData >());
	typeSet.insert(&type_of< SpatialGroupEntityData >());
	return typeSet;
}

Entity* GroupEntityFactory::createEntity(IEntityBuilder* builder, const std::wstring& name, const EntityData& entityData) const
{
	if (const GroupEntityData* groupData = dynamic_type_cast< const GroupEntityData* >(&entityData))
	{
		const RefArray< EntityInstance >& instances = groupData->getInstances();
		Ref< GroupEntity > groupEntity = gc_new< GroupEntity >();
		for (RefArray< EntityInstance >::const_iterator i = instances.begin(); i != instances.end(); ++i)
		{
			Ref< Entity > childEntity = builder->build(*i);
			if (childEntity)
				groupEntity->m_entities.push_back(childEntity);
		}
		return groupEntity;
	}
	else if (const SpatialGroupEntityData* spatialGroupData = dynamic_type_cast< const SpatialGroupEntityData* >(&entityData))
	{
		const RefArray< EntityInstance >& instances = spatialGroupData->getInstances();
		Ref< SpatialGroupEntity > spatialGroupEntity = gc_new< SpatialGroupEntity >(cref(spatialGroupData->getTransform()));
		for (RefArray< EntityInstance >::const_iterator i = instances.begin(); i != instances.end(); ++i)
		{
			Ref< SpatialEntity > childEntity = dynamic_type_cast< SpatialEntity* >(builder->build(*i));
			if (childEntity)
				spatialGroupEntity->m_entities.push_back(childEntity);
		}
		return spatialGroupEntity;
	}
	else
		return 0;
}
	
	}
}
