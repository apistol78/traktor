#include "Database/Database.h"
#include "World/EntityEventResourceFactory.h"
#include "World/EntityEventSet.h"
#include "World/EntityEventSetData.h"
#include "World/IEntityBuilder.h"
#include "World/IEntityEvent.h"
#include "World/IEntityEventData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.EntityEventResourceFactory", EntityEventResourceFactory, resource::IResourceFactory)

EntityEventResourceFactory::EntityEventResourceFactory(db::Database* db, const IEntityBuilder* entityBuilder)
:	m_db(db)
,	m_entityBuilder(entityBuilder)
{
}

const TypeInfoSet EntityEventResourceFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< EntityEventSetData >());
	typeSet.insert(&type_of< IEntityEventData >());
	return typeSet;
}

const TypeInfoSet EntityEventResourceFactory::getProductTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< EntityEventSet >());
	typeSet.insert(&type_of< IEntityEvent >());
	return typeSet;
}

bool EntityEventResourceFactory::isCacheable() const
{
	return true;
}

Ref< Object > EntityEventResourceFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid, const Object* current) const
{
	if (is_type_a< EntityEventSet >(resourceType) || is_type_a< EntityEventSetData >(resourceType))
	{
		Ref< const EntityEventSetData > eventSetData = m_db->getObjectReadOnly< EntityEventSetData >(guid);
		if (!eventSetData)
			return 0;

		Ref< EntityEventSet > eventSet = eventSetData->create(m_entityBuilder);
		if (!eventSet)
			return 0;

		return eventSet;
	}
	else if (is_type_a< IEntityEvent >(resourceType) || is_type_a< IEntityEventData >(resourceType))
	{
		Ref< const IEntityEventData > eventData = m_db->getObjectReadOnly< IEntityEventData >(guid);
		if (!eventData)
			return 0;

		Ref< IEntityEvent > event = m_entityBuilder->create< IEntityEvent >(eventData);
		if (!event)
			return 0;

		return event;
	}
	else
		return 0;
}

	}
}
