#include "World/Entity/GroupEntityFactory.h"
#include "World/Entity/GroupEntityData.h"
#include "World/Entity/GroupEntity.h"
#include "World/Entity/SpatialGroupEntityData.h"
#include "World/Entity/SpatialGroupEntity.h"
#include "World/Entity/EntityInstance.h"
#include "World/Entity/IEntityBuilder.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace world
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.world.GroupEntityFactory", GroupEntityFactory, IEntityFactory)

const TypeInfoSet GroupEntityFactory::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< GroupEntityData >());
	typeSet.insert(&type_of< SpatialGroupEntityData >());
	return typeSet;
}

Ref< Entity > GroupEntityFactory::createEntity(IEntityBuilder* builder, const std::wstring& name, const EntityData& entityData, const Object* instanceData) const
{
	if (const GroupEntityData* groupData = dynamic_type_cast< const GroupEntityData* >(&entityData))
	{
		const RefArray< EntityInstance >& instances = groupData->getInstances();
		Ref< GroupEntity > groupEntity = new GroupEntity();
		for (RefArray< EntityInstance >::const_iterator i = instances.begin(); i != instances.end(); ++i)
		{
			T_FATAL_ASSERT(*i);
			Ref< Entity > childEntity = builder->build(*i);
			if (childEntity)
				groupEntity->m_entities.push_back(childEntity);
			else
				log::error << L"Unable to create entity from instance \"" << (*i)->getName() << L"\"";
		}
		return groupEntity;
	}
	else if (const SpatialGroupEntityData* spatialGroupData = dynamic_type_cast< const SpatialGroupEntityData* >(&entityData))
	{
		const RefArray< EntityInstance >& instances = spatialGroupData->getInstances();
		Ref< SpatialGroupEntity > spatialGroupEntity = new SpatialGroupEntity(spatialGroupData->getTransform());
		for (RefArray< EntityInstance >::const_iterator i = instances.begin(); i != instances.end(); ++i)
		{
			T_FATAL_ASSERT(*i);
			Ref< SpatialEntity > childEntity = dynamic_type_cast< SpatialEntity* >(builder->build(*i));
			if (childEntity)
				spatialGroupEntity->m_entities.push_back(childEntity);
			else
				log::error << L"Unable to create spatial entity from instance \"" << (*i)->getName() << L"\"";
		}
		return spatialGroupEntity;
	}
	else
	{
		T_BREAKPOINT;
		return 0;
	}
}
	
	}
}
